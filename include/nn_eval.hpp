#pragma once

// =============================================================================
// Batu Chess Engine - Neural Network Evaluation
// =============================================================================
//
// Simple feedforward neural network for position evaluation.
// Architecture: 768 -> 256 -> 32 -> 1
// Input: 12 pieces × 64 squares = 768 binary features
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
    
    // Read weights in order: layer1 weights, layer1 bias, layer2 weights, etc.
    
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
// Forward Pass
// =============================================================================

inline int evaluate(const U64* piece_bitboards, int side) {
    if (!nn_loaded) return 0;
    
    // Build input vector from bitboards
    float input[INPUT_SIZE] = {0.0f};
    
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
            input[piece * 64 + square] = 1.0f;
            bb &= bb - 1;  // Clear LSB
        }
    }
    
    // Layer 1: input -> hidden1 (ReLU)
    float hidden1[HIDDEN1_SIZE];
    for (int j = 0; j < HIDDEN1_SIZE; j++) {
        float sum = bias_hidden1[j];
        for (int i = 0; i < INPUT_SIZE; i++) {
            if (input[i] > 0.0f) {  // Sparse optimization
                sum += input[i] * weights_input_hidden1[i * HIDDEN1_SIZE + j];
            }
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
    
    // Layer 3: hidden2 -> output (tanh)
    float output = bias_output;
    for (int i = 0; i < HIDDEN2_SIZE; i++) {
        output += hidden2[i] * weights_hidden2_output[i];
    }
    output = std::tanh(output);
    
    // Convert to centipawns and adjust for side to move
    int score = static_cast<int>(output * SCALE_FACTOR);
    return (side == WHITE) ? score : -score;
}

// =============================================================================
// Initialization
// =============================================================================

inline void init(const std::string& weights_path = "weights.txt") {
    if (load_weights(weights_path)) {
        std::cerr << "Neural network loaded from: " << weights_path << std::endl;
    }
}

} // namespace NN
