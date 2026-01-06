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
#include "include/uci.hpp"

int main() {
    // Initialize attack tables (magic bitboards)
    AttackTables::init_all();

    // Create position and run UCI loop
    Position pos;
    UCI::loop(pos);

    return 0;
}