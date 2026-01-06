#pragma once

// =============================================================================
// Batu Chess Engine - Search
// =============================================================================

#include "position.hpp"
#include "movegen.hpp"
#include "nn_eval.hpp"
#include <algorithm>

// Forward declaration of UCI option
extern bool UseNN;

namespace Search {

// =============================================================================
// Move Ordering
// =============================================================================

inline void order_moves(Position& pos, MoveList& moves) {
    for (int i = 0; i < moves.count; i++) {
        moves.score_guess[i] = 0;
        int move = moves.moves[i];
        int source_piece = get_move_piece(move);
        int target_square = get_move_target(move);
        int promoted = get_move_promoted(move);
        int is_check = get_move_checking(move);
        
        // MVV-LVA for captures
        if (get_move_capture(move)) {
            int target_piece = P;
            int start = (pos.side == WHITE) ? p : P;
            int end = (pos.side == WHITE) ? k : K;
            
            for (int bb_piece = start; bb_piece <= end; bb_piece++) {
                if (get_bit(pos.piece_bitboards[bb_piece], target_square)) {
                    target_piece = bb_piece;
                    break;
                }
            }
            moves.score_guess[i] = -(10 * PIECE_VALUES[target_piece] - PIECE_VALUES[source_piece]);
        }
        
        // Promotion bonus
        if (promoted)
            moves.score_guess[i] += -PIECE_VALUES[promoted];
        
        // Center control bonus
        if (target_square == d4 || target_square == d5 || 
            target_square == e4 || target_square == e5)
            moves.score_guess[i] += PIECE_VALUES[source_piece];
        
        // Development bonus
        if ((source_piece == B && (target_square == b2 || target_square == g2)) ||
            (source_piece == b && (target_square == b7 || target_square == g7)) ||
            (source_piece == N && (target_square == c3 || target_square == f3)) ||
            (source_piece == n && (target_square == c6 || target_square == f6)))
            moves.score_guess[i] += PIECE_VALUES[source_piece];
        
        // Check bonus
        if (is_check)
            moves.score_guess[i] += PIECE_VALUES[source_piece] / 10;
    }
}

inline void sort_moves(MoveList& moves, int is_white) {
    // Bubble sort (simple, works for small lists)
    for (int i = 0; i < moves.count - 1; i++) {
        for (int j = 0; j < moves.count - i - 1; j++) {
            bool should_swap = is_white ? 
                (moves.score_guess[j] < moves.score_guess[j + 1]) :
                (moves.score_guess[j] > moves.score_guess[j + 1]);
            
            if (should_swap) {
                std::swap(moves.moves[j], moves.moves[j + 1]);
                std::swap(moves.scores[j], moves.scores[j + 1]);
                std::swap(moves.legality[j], moves.legality[j + 1]);
                std::swap(moves.score_guess[j], moves.score_guess[j + 1]);
            }
        }
    }
}

// =============================================================================
// Alpha-Beta Search
// =============================================================================

inline int negamax(Position& pos, int depth, int alpha, int beta) {
    if (depth == 0) {
        pos.nodes++;
        if (UseNN && NN::nn_loaded) {
            return NN::evaluate(pos.piece_bitboards, pos.side);
        }
        return pos.evaluate();
    }
    
    MoveList moves;
    pos.generate_moves(moves);
    order_moves(pos, moves);
    sort_moves(moves, pos.side == WHITE);
    
    if (moves.count == 0)
        return 0;  // Stalemate or checkmate handling would go here
    
    for (int i = 0; i < moves.count; i++) {
        Position backup;
        pos.copy_to(backup);
        
        if (!pos.make_move(moves.moves[i], ALL_MOVES))
            continue;
        
        pos.nodes++;
        int score = -negamax(pos, depth - 1, -beta, -alpha);
        
        backup.copy_to(pos);
        
        if (score >= beta)
            return beta;
        
        if (score > alpha)
            alpha = score;
    }
    
    return alpha;
}

inline MoveList search(Position& pos, int depth) {
    MoveList moves;
    pos.generate_moves(moves);
    order_moves(pos, moves);
    sort_moves(moves, pos.side == WHITE);
    
    for (int i = 0; i < moves.count; i++) {
        Position backup;
        pos.copy_to(backup);
        
        if (!pos.make_move(moves.moves[i], ALL_MOVES)) {
            moves.legality[i] = false;
            continue;
        }
        
        moves.legality[i] = true;
        int score = -negamax(pos, depth - 1, -INFINITY_SCORE, INFINITY_SCORE);
        
        backup.copy_to(pos);
        
        moves.scores[i] = (pos.side == WHITE) ? score : -score;
    }
    
    return moves;
}

inline int find_best_move(Position& pos, MoveList& moves) {
    int best_move = 0;
    int best_score = (pos.side == WHITE) ? -INFINITY_SCORE : INFINITY_SCORE;
    
    for (int i = 0; i < moves.count; i++) {
        if (!moves.legality[i]) continue;
        
        bool is_better = (pos.side == WHITE) ? 
            (moves.scores[i] > best_score) :
            (moves.scores[i] < best_score);
        
        if (is_better) {
            best_score = moves.scores[i];
            best_move = moves.moves[i];
        }
    }
    
    return best_move;
}

} // namespace Search
