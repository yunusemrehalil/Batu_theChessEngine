#pragma once

// =============================================================================
// Batu Chess Engine - Neural Network Evaluation
// =============================================================================
//
// Neural network with PSQT (Piece-Square Table) skip connection.
// 
// Architecture:
//     Input (768) ──┬──> PSQT (768->1, linear) ─────────┐
//                   │                                    │
//                   └──> fc1 (768->256, ReLU)           │
//                           │                            │
//                        fc2 (256->32, ReLU)            │
//                           │                            │
//                        fc3 (32->1) ──> positional     │
//                                            │           │
//                                            └──> + <────┘
//                                                 │
//                                              tanh(output)
//
// Key insight: Material values flow directly through linear PSQT layer,
// making it trivial to learn piece values. Deeper layers learn positional
// adjustments only.
//
// =============================================================================

#include "types.hpp"
#include <cmath>
#include <fstream>
#include <string>
#include <iostream>

#ifdef _MSC_VER
#include <intrin.h>
#endif

namespace NN {

// =============================================================================
// Network Architecture Constants
// =============================================================================

constexpr int INPUT_SIZE = 768;     // 12 pieces × 64 squares
constexpr int HIDDEN1_SIZE = 256;
constexpr int HIDDEN2_SIZE = 32;
constexpr int OUTPUT_SIZE = 1;

constexpr int SCALE_FACTOR = 600;   // tanh output × 600 = centipawns

// =============================================================================
// Network Weights (Global)
// =============================================================================

// PSQT skip connection weights (direct material path)
inline float psqt_weights[INPUT_SIZE];

// Positional network weights
inline float weights_input_hidden1[INPUT_SIZE * HIDDEN1_SIZE];
inline float bias_hidden1[HIDDEN1_SIZE];
inline float weights_hidden1_hidden2[HIDDEN1_SIZE * HIDDEN2_SIZE];
inline float bias_hidden2[HIDDEN2_SIZE];
inline float weights_hidden2_output[HIDDEN2_SIZE];
inline float bias_output;

inline bool nn_loaded = false;

// =============================================================================
// Activation Function
// =============================================================================

inline float relu(float x) {
    return x > 0.0f ? x : 0.0f;
}

// =============================================================================
// Weight Loading
// =============================================================================

inline bool load_weights(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return false;
    }
    
    // Read weights in order: PSQT, then layer1, layer2, layer3
    
    // PSQT skip connection weights [768]
    for (int i = 0; i < INPUT_SIZE; i++) {
        if (!(file >> psqt_weights[i])) return false;
    }
    
    // Layer 1: input -> hidden1
    for (int i = 0; i < INPUT_SIZE * HIDDEN1_SIZE; i++) {
        if (!(file >> weights_input_hidden1[i])) return false;
    }
    for (int i = 0; i < HIDDEN1_SIZE; i++) {
        if (!(file >> bias_hidden1[i])) return false;
    }
    
    // Layer 2: hidden1 -> hidden2
    for (int i = 0; i < HIDDEN1_SIZE * HIDDEN2_SIZE; i++) {
        if (!(file >> weights_hidden1_hidden2[i])) return false;
    }
    for (int i = 0; i < HIDDEN2_SIZE; i++) {
        if (!(file >> bias_hidden2[i])) return false;
    }
    
    // Layer 3: hidden2 -> output
    for (int i = 0; i < HIDDEN2_SIZE; i++) {
        if (!(file >> weights_hidden2_output[i])) return false;
    }
    if (!(file >> bias_output)) return false;
    
    file.close();
    nn_loaded = true;
    return true;
}

// =============================================================================
// Forward Pass (Sparse First Layer)
// =============================================================================

// Maximum pieces on board (32 at start, can't exceed this)
constexpr int MAX_ACTIVE_FEATURES = 32;

inline int evaluate(const U64* piece_bitboards, int side) {
    if (!nn_loaded) return 0;
    
    // =========================================================================
    // Step 1: Collect active feature indices (sparse representation)
    // Instead of building a 768-element array, we just store which features are "on"
    // =========================================================================
    int active_features[MAX_ACTIVE_FEATURES];
    int num_active = 0;
    
    for (int piece = 0; piece < 12; piece++) {
        U64 bb = piece_bitboards[piece];
        while (bb) {
            unsigned long idx;
#ifdef _MSC_VER
            _BitScanForward64(&idx, bb);
            int square = static_cast<int>(idx);
#else
            int square = __builtin_ctzll(bb);
#endif
            // Feature index = piece * 64 + square
            active_features[num_active++] = piece * 64 + square;
            bb &= bb - 1;  // Clear LSB
        }
    }
    
    // =========================================================================
    // Step 2: PSQT skip connection (direct material evaluation)
    // Material flows directly through a linear layer, making piece values
    // trivial to learn. Only ~32 additions for active pieces!
    // =========================================================================
    float psqt = 0.0f;
    for (int k = 0; k < num_active; k++) {
        psqt += psqt_weights[active_features[k]];
    }
    
    // =========================================================================
    // Step 3: Layer 1 - Sparse accumulation (only iterate active features)
    // OLD: 256 neurons × 768 checks = 196,608 operations
    // NEW: 256 neurons × ~32 active = ~8,192 operations
    // =========================================================================
    float hidden1[HIDDEN1_SIZE];
    for (int j = 0; j < HIDDEN1_SIZE; j++) {
        float sum = bias_hidden1[j];
        // Only accumulate weights for active features
        for (int k = 0; k < num_active; k++) {
            int feature_idx = active_features[k];
            sum += weights_input_hidden1[feature_idx * HIDDEN1_SIZE + j];
        }
        hidden1[j] = relu(sum);
    }
    
    // Layer 2: hidden1 -> hidden2 (ReLU)
    float hidden2[HIDDEN2_SIZE];
    for (int j = 0; j < HIDDEN2_SIZE; j++) {
        float sum = bias_hidden2[j];
        for (int i = 0; i < HIDDEN1_SIZE; i++) {
            sum += hidden1[i] * weights_hidden1_hidden2[i * HIDDEN2_SIZE + j];
        }
        hidden2[j] = relu(sum);
    }
    
    // Layer 3: hidden2 -> output (positional component)
    float positional = bias_output;
    for (int i = 0; i < HIDDEN2_SIZE; i++) {
        positional += hidden2[i] * weights_hidden2_output[i];
    }
    
    // =========================================================================
    // Combine PSQT (material) + positional, then apply tanh
    // =========================================================================
    float output = std::tanh(psqt + positional);
    
    // Convert to centipawns and adjust for side to move
    int score = static_cast<int>(output * SCALE_FACTOR);
    return (side == WHITE) ? score : -score;
}

// =============================================================================
// Initialization
// =============================================================================

inline void init(const std::string& weights_path = "weights.txt") {
    if (load_weights(weights_path)) {
        std::cout << "Neural network loaded from: " << weights_path << std::endl;
    }
}

} // namespace NN
