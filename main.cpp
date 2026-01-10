// =============================================================================
// Batu Chess Engine - Main Entry Point
// =============================================================================
//
// A UCI-compatible chess engine written in C++
// Author: Yunus Emre Halil
//
// Architecture:
//   - Bitboard representation (magic bitboards for sliding pieces)
//   - Alpha-beta search with move ordering
//   - Static evaluation (prepared for neural network replacement)
//
// =============================================================================

#include "include/position.hpp"
#include "include/attacks.hpp"
#include "include/nn_eval.hpp"
#include "include/tt.hpp"
#include "include/search.hpp"
#include "include/uci.hpp"

int main() {
    // Initialize attack tables (magic bitboards)
    AttackTables::init_all();
    
    // Initialize Zobrist hashing for TT
    TT::init_zobrist();
    TT::clear();
    
    // Initialize LMR reduction table
    Search::init_lmr();
    
    // Initialize neural network (loads weights if available)
    NN::init("weights.txt");

    // Create position and run UCI loop
    Position pos;
    UCI::loop(pos);

    return 0;
}