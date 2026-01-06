#pragma once

// =============================================================================
// Batu Chess Engine - UCI Protocol Implementation
// =============================================================================

#include "position.hpp"
#include "search.hpp"
#include "nn_eval.hpp"
#include <iostream>
#include <cstring>
#include <cstdio>
#include <chrono>

// UCI Options
inline bool UseNN = true;  // Use neural network evaluation when available

namespace UCI {

// =============================================================================
// Move Parsing
// =============================================================================

inline int parse_move(Position& pos, const char* move_string) {
    MoveList moves;
    pos.generate_moves(moves);
    
    int source = (move_string[0] - 'a') + (8 - (move_string[1] - '0')) * 8;
    int target = (move_string[2] - 'a') + (8 - (move_string[3] - '0')) * 8;
    
    for (int i = 0; i < moves.count; i++) {
        int move = moves.moves[i];
        
        if (source == get_move_source(move) && target == get_move_target(move)) {
            int promoted = get_move_promoted(move);
            
            if (promoted) {
                char promo_char = move_string[4];
                if ((promoted == Q || promoted == q) && promo_char == 'q') return move;
                if ((promoted == R || promoted == r) && promo_char == 'r') return move;
                if ((promoted == B || promoted == b) && promo_char == 'b') return move;
                if ((promoted == N || promoted == n) && promo_char == 'n') return move;
                continue;
            }
            return move;
        }
    }
    
    return 0;
}

// =============================================================================
// Position Command
// =============================================================================

inline void parse_position(Position& pos, char* command) {
    command += 9;  // Skip "position "
    
    if (std::strncmp(command, "startpos", 8) == 0) {
        pos.parse_fen(START_POSITION);
    } else {
        char* fen = std::strstr(command, "fen");
        if (fen == nullptr) {
            pos.parse_fen(START_POSITION);
        } else {
            fen += 4;
            pos.parse_fen(fen);
        }
    }
    
    char* moves = std::strstr(command, "moves");
    if (moves != nullptr) {
        moves += 6;
        
        while (*moves) {
            int move = parse_move(pos, moves);
            if (move == 0) break;
            
            pos.make_move(move, ALL_MOVES);
            
            while (*moves && *moves != ' ') moves++;
            moves++;
        }
    }
    
    pos.print();
}

// =============================================================================
// Go Command
// =============================================================================

inline void parse_go(Position& pos, char* command) {
    int depth = 6;  // Default depth
    
    char* depth_str = std::strstr(command, "depth");
    if (depth_str != nullptr) {
        depth = std::atoi(depth_str + 6);
    }
    
    pos.nodes = 0;
    
    auto start = std::chrono::high_resolution_clock::now();
    MoveList moves = Search::search(pos, depth);
    auto end = std::chrono::high_resolution_clock::now();
    
    int best_move = Search::find_best_move(pos, moves);
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "info depth " << depth << " nodes " << pos.nodes 
              << " time " << duration.count() << std::endl;
    std::cout << "bestmove ";
    Position::print_move(best_move);
    std::cout << std::endl;
}

// =============================================================================
// Benchmark Command
// =============================================================================

inline void run_benchmark(Position& pos) {
    const char* positions[] = {
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",  // Starting position
        "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -"  // Position 2
    };
    const char* names[] = { "Starting Position", "Position 2 (Kiwipete)" };
    int depths[] = { 5, 6, 7 };
    
    std::cout << "\n=== BATU CHESS ENGINE BENCHMARK ===" << std::endl;
    std::cout << "With Alpha-Beta Pruning + Move Ordering\n" << std::endl;
    
    for (int p = 0; p < 2; p++) {
        std::cout << "### " << names[p] << " ###" << std::endl;
        std::cout << "FEN: " << positions[p] << "\n" << std::endl;
        
        for (int d = 0; d < 3; d++) {
            pos.parse_fen(positions[p]);
            pos.nodes = 0;
            
            auto start = std::chrono::high_resolution_clock::now();
            MoveList moves = Search::search(pos, depths[d]);
            auto end = std::chrono::high_resolution_clock::now();
            
            int best_move = Search::find_best_move(pos, moves);
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            
            std::cout << "Depth " << depths[d] << ": ";
            std::cout << duration.count() << " ms, ";
            std::cout << pos.nodes << " nodes, ";
            std::cout << "Best: ";
            Position::print_move(best_move);
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }
}

// =============================================================================
// UCI Loop
// =============================================================================

inline void loop(Position& pos) {
    char input[2000];
    
    std::cout << "id name Batu" << std::endl;
    std::cout << "id author Yunus Emre Halil" << std::endl;
    std::cout << "option name UseNN type check default " << (NN::nn_loaded ? "true" : "false") << std::endl;
    std::cout << "uciok" << std::endl;
    
    while (true) {
        std::memset(input, 0, sizeof(input));
        std::fflush(stdout);
        
        if (!std::fgets(input, 2000, stdin))
            continue;
        
        if (input[0] == '\n')
            continue;
        
        if (std::strncmp(input, "isready", 7) == 0) {
            std::cout << "readyok" << std::endl;
            continue;
        }
        
        if (std::strncmp(input, "setoption", 9) == 0) {
            if (std::strstr(input, "UseNN")) {
                UseNN = (std::strstr(input, "true") != nullptr);
            }
            continue;
        }
        
        if (std::strncmp(input, "position", 8) == 0) {
            parse_position(pos, input);
            continue;
        }
        
        if (std::strncmp(input, "ucinewgame", 10) == 0) {
            parse_position(pos, (char*)"position startpos");
            continue;
        }
        
        if (std::strncmp(input, "go", 2) == 0) {
            parse_go(pos, input);
            continue;
        }
        
        if (std::strncmp(input, "eval", 4) == 0) {
            int nn_score = NN::evaluate(pos.piece_bitboards, pos.side);
            int static_score = pos.evaluate();
            std::cout << "info string NN: " << nn_score << " cp, Static: " << static_score << " cp" << std::endl;
            continue;
        }
        
        if (std::strncmp(input, "bench", 5) == 0) {
            run_benchmark(pos);
            continue;
        }
        
        if (std::strncmp(input, "quit", 4) == 0)
            break;
        
        if (std::strncmp(input, "uci", 3) == 0) {
            std::cout << "id name Batu" << std::endl;
            std::cout << "id author Yunus Emre Halil" << std::endl;
            std::cout << "option name UseNN type check default " << (NN::nn_loaded ? "true" : "false") << std::endl;
            std::cout << "uciok" << std::endl;
        }
    }
}

} // namespace UCI
