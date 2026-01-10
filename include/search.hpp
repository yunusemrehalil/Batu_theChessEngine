#pragma once

// =============================================================================
// Batu Chess Engine - Search
// =============================================================================

#include "position.hpp"
#include "movegen.hpp"
#include "nn_eval.hpp"
#include "tt.hpp"
#include <algorithm>

// Forward declaration of UCI option
extern bool UseNN;

namespace Search {

// =============================================================================
// Search Parameters
// =============================================================================

constexpr int MAX_QUIESCENCE_DEPTH = 8;

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

// Pick the best move from position 'start' to end, swap it to 'start'
// This is O(n) per call but we often get cutoffs before searching all moves
inline void pick_best_move(MoveList& moves, int start) {
    int best_idx = start;
    int best_score = moves.score_guess[start];
    
    for (int i = start + 1; i < moves.count; i++) {
        if (moves.score_guess[i] < best_score) {  // Lower = better
            best_score = moves.score_guess[i];
            best_idx = i;
        }
    }
    
    if (best_idx != start) {
        std::swap(moves.moves[start], moves.moves[best_idx]);
        std::swap(moves.scores[start], moves.scores[best_idx]);
        std::swap(moves.legality[start], moves.legality[best_idx]);
        std::swap(moves.score_guess[start], moves.score_guess[best_idx]);
    }
}

inline void sort_moves(MoveList& moves) {
    // Selection sort - O(n²) worst case but simpler than bubble sort
    // In practice with good move ordering + cutoffs, we rarely sort everything
    for (int i = 0; i < moves.count - 1; i++) {
        pick_best_move(moves, i);
    }
}

// =============================================================================
// Quiescence Search - Resolve Tactical Positions
// =============================================================================

inline int quiescence(Position& pos, int alpha, int beta, int ply = 0) {
    // Standing pat score
    int stand_pat = (UseNN && NN::nn_loaded) ? 
                    NN::evaluate(pos.piece_bitboards, pos.side) : 
                    pos.evaluate();
    
    pos.nodes++;
    
    // Depth limit to prevent explosion
    if (ply >= MAX_QUIESCENCE_DEPTH) return stand_pat;
    
    // Beta cutoff (standing pat)
    if (stand_pat >= beta) return beta;
    if (stand_pat > alpha) alpha = stand_pat;
    
    MoveList moves;
    pos.generate_moves(moves);
    order_moves(pos, moves);
    sort_moves(moves);
    
    for (int i = 0; i < moves.count; i++) {
        // Only search captures
        if (!get_move_capture(moves.moves[i])) continue;
        
        Position backup;
        pos.copy_to(backup);
        
        if (!pos.make_move(moves.moves[i], ALL_MOVES)) continue;
        
        int score = -quiescence(pos, -beta, -alpha, ply + 1);
        backup.copy_to(pos);
        
        if (score >= beta) return beta;
        if (score > alpha) alpha = score;
    }
    
    return alpha;
}

// =============================================================================
// Alpha-Beta Search with TT
// =============================================================================

inline int negamax(Position& pos, int depth, int alpha, int beta, int ply = 0) {
    // Generate hash key for TT
    U64 hash_key = TT::generate_hash_key(pos);
    int tt_score, tt_move = 0;
    
    // TT probe: check if we've seen this position before
    if (TT::probe(hash_key, depth, ply, alpha, beta, tt_score, tt_move)) {
        return tt_score;
    }
    
    if (depth == 0) {
        return quiescence(pos, alpha, beta);
    }
    
    int original_alpha = alpha;
    MoveList moves;
    pos.generate_moves(moves);
    order_moves(pos, moves);
    
    // TT move ordering: if we have a TT move, boost its priority
    if (tt_move != 0) {
        for (int i = 0; i < moves.count; i++) {
            if (moves.moves[i] == tt_move) {
                moves.score_guess[i] = -1000000;  // Highest priority
                break;
            }
        }
    }
    
    sort_moves(moves);
    
    int legal_moves = 0;
    int best_move = 0;
    int best_score = -INFINITY_SCORE;
    
    for (int i = 0; i < moves.count; i++) {
        Position backup;
        pos.copy_to(backup);
        
        if (!pos.make_move(moves.moves[i], ALL_MOVES))
            continue;
        
        legal_moves++;
        pos.nodes++;
        int score = -negamax(pos, depth - 1, -beta, -alpha, ply + 1);
        
        backup.copy_to(pos);
        
        if (score > best_score) {
            best_score = score;
            best_move = moves.moves[i];
        }
        
        if (score >= beta) {
            // Store with BETA flag (lower bound - failed high)
            TT::store(hash_key, beta, depth, ply, TT::TT_BETA, best_move);
            return beta;
        }
        
        if (score > alpha)
            alpha = score;
    }
    
    // Checkmate or stalemate detection
    if (legal_moves == 0) {
        int king_sq = Position::get_ls1b_index(pos.piece_bitboards[pos.side == WHITE ? K : k]);
        if (pos.is_square_attacked(king_sq, pos.side ^ 1))
            return -CHECKMATE_SCORE + ply;  // Checkmate: prefer faster mates
        return STALEMATE_SCORE;  // Stalemate
    }
    
    // Store result in TT
    TT::TTFlag flag = (alpha > original_alpha) ? TT::TT_EXACT : TT::TT_ALPHA;
    TT::store(hash_key, alpha, depth, ply, flag, best_move);
    
    return alpha;
}

inline MoveList search(Position& pos, int depth) {
    MoveList moves;
    pos.generate_moves(moves);
    order_moves(pos, moves);
    sort_moves(moves);
    
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
