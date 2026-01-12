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
// Go Command - Adaptive Time Management
// =============================================================================

inline void parse_go(Position& pos, char* command) {
    int max_depth = 64;
    int optimal_time = 0;   // Target time to use
    int maximum_time = 0;   // Hard limit (don't exceed)
    int our_time = 0;
    int our_inc = 0;
    int moves_to_go = 0;
    constexpr int MOVE_OVERHEAD = 50;  // Safety margin in ms
    
    // Parse depth limit
    char* depth_str = std::strstr(command, "depth");
    if (depth_str != nullptr) {
        max_depth = std::atoi(depth_str + 6);
    }
    
    // Parse movetime (fixed time per move)
    char* movetime_str = std::strstr(command, "movetime");
    if (movetime_str != nullptr) {
        optimal_time = std::atoi(movetime_str + 9);
        maximum_time = optimal_time;
    }
    
    // Parse movestogo (moves until next time control)
    char* mtg_str = std::strstr(command, "movestogo");
    if (mtg_str != nullptr) {
        moves_to_go = std::atoi(mtg_str + 10);
    }
    
    // Parse wtime/btime (remaining time on clock)
    char* wtime_str = std::strstr(command, "wtime");
    char* btime_str = std::strstr(command, "btime");
    
    // Parse winc/binc (increment per move)
    char* winc_str = std::strstr(command, "winc");
    char* binc_str = std::strstr(command, "binc");
    
    if (pos.side == WHITE) {
        if (wtime_str) our_time = std::atoi(wtime_str + 6);
        if (winc_str) our_inc = std::atoi(winc_str + 5);
    } else {
        if (btime_str) our_time = std::atoi(btime_str + 6);
        if (binc_str) our_inc = std::atoi(binc_str + 5);
    }
    
    // Calculate time for this move (only if not already set by movetime)
    if (our_time > 0 && optimal_time == 0) {
        if (moves_to_go > 0) {
            // Tournament time control: x moves in y minutes
            // Distribute time evenly with increment bonus
            int time_for_moves = our_time + our_inc * (moves_to_go - 1) - MOVE_OVERHEAD * moves_to_go;
            optimal_time = std::max(10, time_for_moves / moves_to_go);
            maximum_time = std::min(our_time - MOVE_OVERHEAD, optimal_time * 3);
        } else {
            // Sudden death or increment time control
            // Adaptive scaling based on remaining time
            int base_moves = 30;  // Assume ~30 moves left on average
            
            // Scale based on remaining time (use more time early, less when low)
            if (our_time > 60000) {
                base_moves = 40;  // Plenty of time, be conservative
            } else if (our_time < 10000) {
                base_moves = 20;  // Low on time, move faster
            } else if (our_time < 5000) {
                base_moves = 15;  // Very low, panic mode
            }
            
            // Calculate optimal time with increment bonus
            optimal_time = our_time / base_moves + our_inc * 3 / 4;
            
            // Maximum time: can extend up to 2.5x optimal, but capped
            maximum_time = optimal_time * 5 / 2;
            
            // Never use more than 20% of remaining time in one move
            maximum_time = std::min(maximum_time, our_time / 5);
            
            // Safety: always leave some buffer
            maximum_time = std::min(maximum_time, our_time - MOVE_OVERHEAD);
            optimal_time = std::min(optimal_time, maximum_time);
        }
        
        // Minimum time bounds
        optimal_time = std::max(optimal_time, 10);
        maximum_time = std::max(maximum_time, optimal_time);
    }
    
    // Parse infinite (no time limit, just depth)
    if (std::strstr(command, "infinite")) {
        optimal_time = 0;
        maximum_time = 0;
    }
    
    pos.nodes = 0;
    int best_move = 0;
    int best_score = 0;
    int prev_best_move = 0;
    int prev_score = 0;
    int stable_count = 0;
    auto start = std::chrono::high_resolution_clock::now();
    
    // Clear killer moves for new search
    Search::clear_killers();
    
    // Iterative deepening loop
    for (int depth = 1; depth <= max_depth; depth++) {
        MoveList moves = Search::search(pos, depth);
        best_move = Search::find_best_move(pos, moves);
        
        // Find the score of best move
        for (int i = 0; i < moves.count; i++) {
            if (moves.moves[i] == best_move) {
                best_score = moves.scores[i];
                break;
            }
        }
        
        // Track best move stability for time management
        if (best_move == prev_best_move) {
            stable_count++;
        } else {
            stable_count = 0;
        }
        prev_best_move = best_move;
        
        auto now = std::chrono::high_resolution_clock::now();
        auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();
        
        // Output info
        std::cout << "info depth " << depth 
                  << " score cp " << best_score
                  << " nodes " << pos.nodes 
                  << " time " << elapsed_ms;
        if (elapsed_ms > 0) {
            std::cout << " nps " << (pos.nodes * 1000 / elapsed_ms);
        }
        std::cout << std::endl;
        
        // Time management decisions
        if (optimal_time > 0) {
            // Hard stop: never exceed maximum time
            if (elapsed_ms >= maximum_time) {
                break;
            }
            
            // Calculate adjusted optimal time based on:
            // 1. Best move stability (stable = use less time)
            // 2. Score drop (dropping = use more time)
            double time_factor = 1.0;
            
            // Stability factor: if best move is stable, reduce time
            if (stable_count >= 4) {
                time_factor *= 0.6;  // Very stable, save time
            } else if (stable_count >= 2) {
                time_factor *= 0.8;  // Somewhat stable
            }
            
            // Score drop factor: if score is falling, use more time
            if (depth > 1) {
                int score_drop = prev_score - best_score;
                if (score_drop > 50) {
                    time_factor *= 1.3;  // Significant drop, think more
                } else if (score_drop > 20) {
                    time_factor *= 1.15;
                }
            }
            prev_score = best_score;
            
            int adjusted_optimal = static_cast<int>(optimal_time * time_factor);
            
            // Stop if we've exceeded adjusted optimal time
            if (elapsed_ms >= adjusted_optimal) {
                break;
            }
            
            // Estimate if next iteration would exceed maximum
            // (branching factor ~2-3, next depth takes ~2.5x current)
            if (depth >= 4 && elapsed_ms * 3 > maximum_time) {
                break;
            }
        }
    }
    
    std::cout << "bestmove ";
    Position::print_move(best_move);
    std::cout << std::endl;
}

// =============================================================================
// Benchmark Command
// =============================================================================

inline void run_benchmark(Position& pos) {
    struct BenchPosition {
        const char* name;
        const char* fen;
        int depth;
        bool expect_mate;  // true if position should find mate
    };
    
    // Positions to test: start position + puzzles
    BenchPosition positions[] = {
        { "Starting Position", START_POSITION, 7, false },
        { "Eval Test (SF: -6.0)", BENCH_EVAL_TEST, 6, false },
        { "Mate in 3", BENCH_MATE_IN_3, 10, true },
        { "Mate in 2", BENCH_MATE_IN_2_B, 6, true }
    };
    int num_positions = sizeof(positions) / sizeof(positions[0]);
    
    std::cout << "\n=== BATU CHESS ENGINE BENCHMARK ===" << std::endl;
    std::cout << "Config: Alpha-Beta + TT + NMP + LMR + Killers";
    std::cout << (UseNN && NN::nn_loaded ? " + NN Eval" : " + Static Eval") << "\n" << std::endl;
    
    // Table header
    std::cout << "| Position             | Depth | Time(ms) | Nodes      | Score  | Best Move | Status |" << std::endl;
    std::cout << "|----------------------|-------|----------|------------|--------|-----------|--------|" << std::endl;
    
    long long total_nodes = 0;
    long long total_time = 0;
    int passed = 0;
    
    for (int i = 0; i < num_positions; i++) {
        // Clear state (same as ucinewgame)
        TT::clear();
        Search::clear_killers();
        
        pos.parse_fen(positions[i].fen);
        pos.nodes = 0;
        
        auto start = std::chrono::high_resolution_clock::now();
        
        // Iterative deepening (same as parse_go)
        int best_move = 0;
        int best_score = 0;
        for (int depth = 1; depth <= positions[i].depth; depth++) {
            MoveList moves = Search::search(pos, depth);
            best_move = Search::find_best_move(pos, moves);
            for (int j = 0; j < moves.count; j++) {
                if (moves.moves[j] == best_move) {
                    best_score = moves.scores[j];
                    break;
                }
            }
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        
        total_nodes += pos.nodes;
        total_time += ms;
        
        // Invariant checks:
        // 1. Best move must be non-zero (engine didn't crash)
        // 2. If expect_mate, score should be near CHECKMATE_SCORE
        bool move_ok = (best_move != 0);
        bool mate_ok = !positions[i].expect_mate || (std::abs(best_score) > CHECKMATE_SCORE - 100);
        bool test_passed = move_ok && mate_ok;
        if (test_passed) passed++;
        
        // Format score (show "mate N" for mate scores)
        char score_str[16];
        if (std::abs(best_score) > CHECKMATE_SCORE - 100) {
            int mate_in = (CHECKMATE_SCORE - std::abs(best_score) + 1) / 2;
            std::snprintf(score_str, sizeof(score_str), "M%d", best_score > 0 ? mate_in : -mate_in);
        } else {
            std::snprintf(score_str, sizeof(score_str), "%d", best_score);
        }
        
        // Format best move
        char move_str[8] = {0};
        std::snprintf(move_str, sizeof(move_str), "%s%s%c",
            SQUARE_TO_COORD[get_move_source(best_move)],
            SQUARE_TO_COORD[get_move_target(best_move)],
            promoted_to_char(get_move_promoted(best_move)));
        
        // Print table row
        std::printf("| %-20s | %5d | %8lld | %10ld | %6s | %-9s | %s |\n",
            positions[i].name, positions[i].depth, (long long)ms, pos.nodes,
            score_str, move_str, test_passed ? "PASS" : "FAIL");
    }
    
    std::cout << "|----------------------|-------|----------|------------|--------|-----------|--------|" << std::endl;
    
    // Summary
    std::cout << "\nSummary: " << passed << "/" << num_positions << " passed";
    std::cout << ", Total: " << total_time << " ms, " << total_nodes << " nodes";
    if (total_time > 0) {
        std::cout << ", " << (total_nodes * 1000 / total_time) << " nps";
    }
    std::cout << std::endl;
    
    // TT reuse test: run first position again, should be faster
    std::cout << "\nTT Reuse Test (re-run pos 1 without clearing TT):" << std::endl;
    pos.parse_fen(positions[0].fen);
    pos.nodes = 0;
    Search::clear_killers();  // Clear killers but NOT TT
    
    auto start2 = std::chrono::high_resolution_clock::now();
    int best_move2 = 0;
    for (int depth = 1; depth <= positions[0].depth; depth++) {
        MoveList moves = Search::search(pos, depth);
        best_move2 = Search::find_best_move(pos, moves);
    }
    auto end2 = std::chrono::high_resolution_clock::now();
    auto ms2 = std::chrono::duration_cast<std::chrono::milliseconds>(end2 - start2).count();
    
    std::cout << "  First run: " << total_time << " ms (for all positions)" << std::endl;
    std::cout << "  TT reuse run (pos 1 only): " << ms2 << " ms, " << pos.nodes << " nodes" << std::endl;
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
            // Clear search state for new game
            TT::clear();
            Search::clear_killers();
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
