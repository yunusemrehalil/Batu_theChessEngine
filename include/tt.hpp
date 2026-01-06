#pragma once

// =============================================================================
// Batu Chess Engine - Transposition Table
// =============================================================================
//
// Minimal TT with Zobrist hashing. Features:
// - 64-bit Zobrist keys for position hashing
// - Always-replace with depth preference
// - Proper bound types (EXACT, ALPHA, BETA)
// - Mate score adjustment for ply distance
//
// =============================================================================

#include "types.hpp"
#include <cstring>

namespace TT {

// =============================================================================
// Zobrist Hashing
// =============================================================================

// Random numbers for Zobrist hashing
inline U64 piece_keys[12][64];     // [piece][square]
inline U64 enpassant_keys[64];     // [square]
inline U64 castling_keys[16];      // [castling rights combination]
inline U64 side_key;               // XOR when black to move

inline bool zobrist_initialized = false;

// Simple PRNG for generating Zobrist keys (xorshift64)
inline U64 random_u64() {
    static U64 seed = 1070372ull;
    seed ^= seed >> 12;
    seed ^= seed << 25;
    seed ^= seed >> 27;
    return seed * 2685821657736338717ull;
}

inline void init_zobrist() {
    if (zobrist_initialized) return;
    
    for (int piece = 0; piece < 12; piece++) {
        for (int square = 0; square < 64; square++) {
            piece_keys[piece][square] = random_u64();
        }
    }
    
    for (int square = 0; square < 64; square++) {
        enpassant_keys[square] = random_u64();
    }
    
    for (int i = 0; i < 16; i++) {
        castling_keys[i] = random_u64();
    }
    
    side_key = random_u64();
    zobrist_initialized = true;
}

// Generate hash key from position state
template<typename Position>
inline U64 generate_hash_key(const Position& pos) {
    U64 key = 0ULL;
    
    // Hash pieces
    for (int piece = 0; piece < 12; piece++) {
        U64 bb = pos.piece_bitboards[piece];
        while (bb) {
            int square = Position::get_ls1b_index(bb);
            key ^= piece_keys[piece][square];
            bb &= bb - 1;
        }
    }
    
    // Hash en passant
    if (pos.enpassant != NO_SQUARE) {
        key ^= enpassant_keys[pos.enpassant];
    }
    
    // Hash castling rights
    key ^= castling_keys[pos.castling];
    
    // Hash side to move
    if (pos.side == BLACK) {
        key ^= side_key;
    }
    
    return key;
}

// =============================================================================
// Transposition Table Entry
// =============================================================================

enum TTFlag : uint8_t {
    TT_EXACT = 0,   // Exact score (PV node)
    TT_ALPHA = 1,   // Upper bound (failed low, score <= alpha)
    TT_BETA  = 2    // Lower bound (failed high, score >= beta)
};

struct TTEntry {
    U64 key;        // Zobrist key for verification
    int score;      // Search score
    int depth;      // Search depth
    int best_move;  // Best move found (for move ordering)
    TTFlag flag;    // Bound type
};

// =============================================================================
// Transposition Table
// =============================================================================

// TT size: 2^20 entries = ~32MB (each entry is ~24 bytes with padding)
constexpr int TT_SIZE_BITS = 20;
constexpr int TT_SIZE = 1 << TT_SIZE_BITS;
constexpr int TT_MASK = TT_SIZE - 1;

inline TTEntry tt_table[TT_SIZE];

inline void clear() {
    std::memset(tt_table, 0, sizeof(tt_table));
}

// =============================================================================
// Mate Score Adjustment
// =============================================================================

// When storing: convert ply-relative mate score to root-relative
inline int score_to_tt(int score, int ply) {
    if (score > CHECKMATE_SCORE - 100) return score + ply;   // We're mating
    if (score < -CHECKMATE_SCORE + 100) return score - ply;  // We're getting mated
    return score;
}

// When retrieving: convert root-relative mate score to ply-relative
inline int score_from_tt(int score, int ply) {
    if (score > CHECKMATE_SCORE - 100) return score - ply;
    if (score < -CHECKMATE_SCORE + 100) return score + ply;
    return score;
}

// =============================================================================
// TT Operations
// =============================================================================

inline void store(U64 key, int score, int depth, int ply, TTFlag flag, int best_move = 0) {
    TTEntry& entry = tt_table[key & TT_MASK];
    
    // Always replace if:
    // - Different position (collision)
    // - Same/deeper depth (more valuable search)
    // - Exact score (most valuable)
    if (entry.key != key || depth >= entry.depth || flag == TT_EXACT) {
        entry.key = key;
        entry.score = score_to_tt(score, ply);
        entry.depth = depth;
        entry.flag = flag;
        if (best_move != 0) entry.best_move = best_move;
    }
}

// Returns: {found, score, best_move}
// Only returns valid score if depth is sufficient and bounds match
inline bool probe(U64 key, int depth, int ply, int alpha, int beta, int& score, int& best_move) {
    TTEntry& entry = tt_table[key & TT_MASK];
    
    if (entry.key != key) return false;
    
    // Always return best move for move ordering, even if depth insufficient
    best_move = entry.best_move;
    
    // Only use score if depth is sufficient
    if (entry.depth < depth) return false;
    
    int tt_score = score_from_tt(entry.score, ply);
    
    switch (entry.flag) {
        case TT_EXACT:
            score = tt_score;
            return true;
            
        case TT_ALPHA:
            // Upper bound: if tt_score <= alpha, we can use cutoff
            if (tt_score <= alpha) {
                score = alpha;
                return true;
            }
            break;
            
        case TT_BETA:
            // Lower bound: if tt_score >= beta, we can use cutoff
            if (tt_score >= beta) {
                score = beta;
                return true;
            }
            break;
    }
    
    return false;
}

// Get TT move for move ordering (doesn't require depth match)
inline int get_tt_move(U64 key) {
    TTEntry& entry = tt_table[key & TT_MASK];
    return (entry.key == key) ? entry.best_move : 0;
}

} // namespace TT
