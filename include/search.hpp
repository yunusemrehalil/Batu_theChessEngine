#pragma once

// =============================================================================
// Batu Chess Engine - Search
// =============================================================================

#include "position.hpp"
#include "movegen.hpp"
#include "nn_eval.hpp"
#include "tt.hpp"
#include <algorithm>
#include <cmath>

// Forward declaration of UCI option
extern bool UseNN;

namespace Search {

// =============================================================================
// Search Parameters
// =============================================================================

constexpr int MAX_QUIESCENCE_DEPTH = 8;
constexpr int MAX_PLY = 64;

// =============================================================================
// Killer Moves Table
// Two killer moves per ply - quiet moves that caused beta cutoffs
// =============================================================================

inline int killer_moves[MAX_PLY][2];

inline void clear_killers() {
    for (int ply = 0; ply < MAX_PLY; ply++) {
        killer_moves[ply][0] = 0;
        killer_moves[ply][1] = 0;
    }
}

inline void store_killer(int move, int ply) {
    // Don't store captures as killers (they're already ordered by MVV-LVA)
    if (get_move_capture(move)) return;
    if (ply >= MAX_PLY) return;
    
    // Shift: move slot 0 -> slot 1, new move -> slot 0
    if (killer_moves[ply][0] != move) {
        killer_moves[ply][1] = killer_moves[ply][0];
        killer_moves[ply][0] = move;
    }
}

inline bool is_killer(int move, int ply) {
    if (ply >= MAX_PLY) return false;
    return (move == killer_moves[ply][0] || move == killer_moves[ply][1]);
}

// =============================================================================
// LMR Reduction Table (precomputed log-based reductions)
// =============================================================================

inline int lmr_table[MAX_PLY][MAX_MOVES];
inline bool lmr_initialized = false;

inline void init_lmr() {
    if (lmr_initialized) return;
    
    for (int depth = 1; depth < MAX_PLY; depth++) {
        for (int move_count = 1; move_count < MAX_MOVES; move_count++) {
            // Classic LMR formula: reduction based on depth and move count
            // R = log(depth) * log(moveCount) / 2
            lmr_table[depth][move_count] = static_cast<int>(
                0.75 + std::log(depth) * std::log(move_count) / 2.25
            );
        }
    }
    lmr_initialized = true;
}

// =============================================================================
// Move Ordering
// =============================================================================

inline void order_moves(Position& pos, MoveList& moves, int ply = 0) {
    for (int i = 0; i < moves.count; i++) {
        moves.score_guess[i] = 0;
        int move = moves.moves[i];
        int source_piece = get_move_piece(move);
        int target_square = get_move_target(move);
        int promoted = get_move_promoted(move);
        int is_check = get_move_checking(move);
        
        // MVV-LVA for captures (highest priority after TT move)
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
            // Captures: -900000 to -800000 range (lower = better)
            moves.score_guess[i] = -900000 + (10 * PIECE_VALUES[target_piece] - PIECE_VALUES[source_piece]);
        }
        // Killer moves (high priority for quiet moves)
        else if (is_killer(move, ply)) {
            // Killers: -700000 range
            moves.score_guess[i] = (move == killer_moves[ply][0]) ? -700000 : -600000;
        }
        // Quiet moves
        else {
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

// Delta margin for promotions (queen - pawn value)
constexpr int DELTA_MARGIN = 200;

// Piece values for delta pruning (absolute values, indexed by piece type)
constexpr int CAPTURE_VALUES[12] = {
    100, 500, 300, 320, 1000, 10000,  // White: P, R, N, B, Q, K
    100, 500, 300, 320, 1000, 10000   // Black: p, r, n, b, q, k (same absolute values)
};

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
        int move = moves.moves[i];
        
        // Only search captures
        if (!get_move_capture(move)) continue;
        
        // =====================================================================
        // Delta Pruning: skip captures that can't raise alpha
        // If stand_pat + captured_piece + margin < alpha, this capture is futile
        // =====================================================================
        int target_sq = get_move_target(move);
        int captured_piece = NO_PIECE;
        int start = (pos.side == WHITE) ? p : P;
        int end = (pos.side == WHITE) ? k : K;
        
        for (int pc = start; pc <= end; pc++) {
            if (get_bit(pos.piece_bitboards[pc], target_sq)) {
                captured_piece = pc;
                break;
            }
        }
        
        // Skip if capture can't raise alpha (with margin for promotions)
        if (captured_piece != NO_PIECE) {
            int gain = CAPTURE_VALUES[captured_piece];
            if (stand_pat + gain + DELTA_MARGIN < alpha) continue;
        }
        
        Position backup;
        pos.copy_to(backup);
        
        if (!pos.make_move(move, ALL_MOVES)) continue;
        
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

inline int negamax(Position& pos, int depth, int alpha, int beta, int ply = 0, bool do_null = true) {
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
    
    // Check detection (needed for NMP safety and checkmate detection)
    int king_sq = Position::get_ls1b_index(pos.piece_bitboards[pos.side == WHITE ? K : k]);
    bool in_check = pos.is_square_attacked(king_sq, pos.side ^ 1);
    
    // =========================================================================
    // Null Move Pruning (NMP)
    // Safety gates: not in check, eval >= beta, non-pawn material, no mate scores
    // =========================================================================
    if (do_null && !in_check && depth >= 3 && std::abs(beta) < CHECKMATE_SCORE - 100) {
        // Get static eval for the safety gate
        int eval = (UseNN && NN::nn_loaded) ? 
                   NN::evaluate(pos.piece_bitboards, pos.side) : pos.evaluate();
        
        // Only try NMP if position looks good (eval >= beta)
        if (eval >= beta) {
            // Need non-pawn material to avoid zugzwang
            U64 pieces = (pos.side == WHITE) 
                ? (pos.piece_bitboards[N] | pos.piece_bitboards[B] | pos.piece_bitboards[R] | pos.piece_bitboards[Q])
                : (pos.piece_bitboards[n] | pos.piece_bitboards[b] | pos.piece_bitboards[r] | pos.piece_bitboards[q]);
            
            if (pieces) {
                Position backup;
                pos.copy_to(backup);
                pos.side ^= 1;
                pos.enpassant = NO_SQUARE;
                
                // R=3 reduction, depth floor at 1
                int score = -negamax(pos, std::max(1, depth - 4), -beta, -beta + 1, ply + 1, false);
                backup.copy_to(pos);
                
                if (score >= beta) return beta;
            }
        }
    }
    
    int original_alpha = alpha;
    MoveList moves;
    pos.generate_moves(moves);
    order_moves(pos, moves, ply);
    
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
        int move = moves.moves[i];
        Position backup;
        pos.copy_to(backup);
        
        if (!pos.make_move(move, ALL_MOVES))
            continue;
        
        legal_moves++;
        pos.nodes++;
        
        int score;
        
        // =====================================================================
        // Late Move Reductions (LMR)
        // Reduce search depth for late quiet moves that are unlikely to be good
        // Safety gates: not in check, depth >= 3, move >= 4, not a capture/promotion
        // =====================================================================
        bool is_quiet = !get_move_capture(move) && !get_move_promoted(move);
        bool can_reduce = !in_check && depth >= 3 && legal_moves >= 4 && is_quiet;
        
        if (can_reduce) {
            // Get reduction from precomputed table
            int reduction = lmr_table[std::min(depth, MAX_PLY - 1)][std::min(legal_moves, MAX_MOVES - 1)];
            
            // Reduce killer moves less (they proved good at this ply)
            if (is_killer(move, ply)) reduction = std::max(0, reduction - 1);
            
            // Ensure we don't reduce below depth 1
            int reduced_depth = std::max(1, depth - 1 - reduction);
            
            // Reduced-depth search with null window
            score = -negamax(pos, reduced_depth, -alpha - 1, -alpha, ply + 1);
            
            // If reduced search fails high, re-search at full depth
            if (score > alpha) {
                score = -negamax(pos, depth - 1, -beta, -alpha, ply + 1);
            }
        } else {
            // Full-depth search for early moves, captures, promotions, and when in check
            score = -negamax(pos, depth - 1, -beta, -alpha, ply + 1);
        }
        
        backup.copy_to(pos);
        
        if (score > best_score) {
            best_score = score;
            best_move = move;
        }
        
        if (score >= beta) {
            // Store killer move for quiet moves that cause beta cutoffs
            store_killer(move, ply);
            
            // Store with BETA flag (lower bound - failed high)
            TT::store(hash_key, beta, depth, ply, TT::TT_BETA, best_move);
            return beta;
        }
        
        if (score > alpha)
            alpha = score;
    }
    
    // Checkmate or stalemate detection (reuse in_check from above)
    if (legal_moves == 0) {
        if (in_check)
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
    order_moves(pos, moves, 0);  // Root level = ply 0
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
