#pragma once

// =============================================================================
// Batu Chess Engine - UCI Protocol Implementation
// =============================================================================

#include "position.hpp"
#include "search.hpp"
#include <iostream>
#include <cstring>
#include <cstdio>

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
    MoveList moves = Search::search(pos, depth);
    int best_move = Search::find_best_move(pos, moves);
    
    std::cout << "bestmove ";
    Position::print_move(best_move);
    std::cout << std::endl;
}

// =============================================================================
// UCI Loop
// =============================================================================

inline void loop(Position& pos) {
    char input[2000];
    
    std::cout << "id name Batu" << std::endl;
    std::cout << "id author Yunus Emre Halil" << std::endl;
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
        
        if (std::strncmp(input, "quit", 4) == 0)
            break;
        
        if (std::strncmp(input, "uci", 3) == 0) {
            std::cout << "id name Batu" << std::endl;
            std::cout << "id author Yunus Emre Halil" << std::endl;
            std::cout << "uciok" << std::endl;
        }
    }
}

} // namespace UCI
