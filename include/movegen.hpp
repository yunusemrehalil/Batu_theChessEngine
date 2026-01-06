#pragma once

// =============================================================================
// Batu Chess Engine - Move Generation Implementation
// =============================================================================

#include "position.hpp"

// =============================================================================
// Move Generation
// =============================================================================

inline void Position::generate_moves(MoveList& moves) const {
    moves.count = 0;
    
    int source, target;
    int enemy_king = (side == WHITE) ? k : K;
    U64 bb, attacks;
    
    for (int piece = P; piece <= k; piece++) {
        bb = piece_bitboards[piece];
        
        // White pawns
        if (side == WHITE && piece == P) {
            while (bb) {
                source = get_ls1b_index(bb);
                target = source - 8;
                
                // Quiet pawn moves
                if (target >= a8 && !get_bit(occupancy[BOTH], target)) {
                    // Promotion
                    if (source >= a7 && source <= h7) {
                        moves.add(encode_move(source, target, piece, Q, 0, 0, 0, 0, 0));
                        moves.add(encode_move(source, target, piece, R, 0, 0, 0, 0, 0));
                        moves.add(encode_move(source, target, piece, B, 0, 0, 0, 0, 0));
                        moves.add(encode_move(source, target, piece, N, 0, 0, 0, 0, 0));
                    } else {
                        // Single push (with check detection)
                        int is_check = (piece_bitboards[k] & pawn_attacks[WHITE][target]) ? 1 : 0;
                        moves.add(encode_move(source, target, piece, 0, 0, 0, 0, 0, is_check));
                        
                        // Double push
                        if (source >= a2 && source <= h2 && !get_bit(occupancy[BOTH], (target - 8))) {
                            int is_check2 = (piece_bitboards[k] & pawn_attacks[WHITE][target - 8]) ? 1 : 0;
                            moves.add(encode_move(source, target - 8, piece, 0, 0, 1, 0, 0, is_check2));
                        }
                    }
                }
                
                // Pawn captures
                attacks = pawn_attacks[WHITE][source] & occupancy[BLACK];
                while (attacks) {
                    target = get_ls1b_index(attacks);
                    if (source >= a7 && source <= h7) {
                        moves.add(encode_move(source, target, piece, Q, 1, 0, 0, 0, 0));
                        moves.add(encode_move(source, target, piece, R, 1, 0, 0, 0, 0));
                        moves.add(encode_move(source, target, piece, B, 1, 0, 0, 0, 0));
                        moves.add(encode_move(source, target, piece, N, 1, 0, 0, 0, 0));
                    } else {
                        int is_check = (piece_bitboards[k] & pawn_attacks[WHITE][target]) ? 1 : 0;
                        moves.add(encode_move(source, target, piece, 0, 1, 0, 0, 0, is_check));
                    }
                    pop_bit(attacks, target);
                }
                
                // En passant
                if (enpassant != NO_SQUARE) {
                    U64 ep_attacks = pawn_attacks[WHITE][source] & (1ULL << enpassant);
                    if (ep_attacks) {
                        target = get_ls1b_index(ep_attacks);
                        int is_check = (piece_bitboards[k] & pawn_attacks[WHITE][target]) ? 1 : 0;
                        moves.add(encode_move(source, target, piece, 0, 1, 0, 1, 0, is_check));
                    }
                }
                
                pop_bit(bb, source);
            }
        }
        
        // Black pawns
        if (side == BLACK && piece == p) {
            while (bb) {
                source = get_ls1b_index(bb);
                target = source + 8;
                
                if (target <= h1 && !get_bit(occupancy[BOTH], target)) {
                    if (source >= a2 && source <= h2) {
                        moves.add(encode_move(source, target, piece, q, 0, 0, 0, 0, 0));
                        moves.add(encode_move(source, target, piece, r, 0, 0, 0, 0, 0));
                        moves.add(encode_move(source, target, piece, b, 0, 0, 0, 0, 0));
                        moves.add(encode_move(source, target, piece, n, 0, 0, 0, 0, 0));
                    } else {
                        int is_check = (piece_bitboards[K] & pawn_attacks[BLACK][target]) ? 1 : 0;
                        moves.add(encode_move(source, target, piece, 0, 0, 0, 0, 0, is_check));
                        
                        if (source >= a7 && source <= h7 && !get_bit(occupancy[BOTH], (target + 8))) {
                            int is_check2 = (piece_bitboards[K] & pawn_attacks[BLACK][target + 8]) ? 1 : 0;
                            moves.add(encode_move(source, target + 8, piece, 0, 0, 1, 0, 0, is_check2));
                        }
                    }
                }
                
                attacks = pawn_attacks[BLACK][source] & occupancy[WHITE];
                while (attacks) {
                    target = get_ls1b_index(attacks);
                    if (source >= a2 && source <= h2) {
                        moves.add(encode_move(source, target, piece, q, 1, 0, 0, 0, 0));
                        moves.add(encode_move(source, target, piece, r, 1, 0, 0, 0, 0));
                        moves.add(encode_move(source, target, piece, b, 1, 0, 0, 0, 0));
                        moves.add(encode_move(source, target, piece, n, 1, 0, 0, 0, 0));
                    } else {
                        int is_check = (piece_bitboards[K] & pawn_attacks[BLACK][target]) ? 1 : 0;
                        moves.add(encode_move(source, target, piece, 0, 1, 0, 0, 0, is_check));
                    }
                    pop_bit(attacks, target);
                }
                
                if (enpassant != NO_SQUARE) {
                    U64 ep_attacks = pawn_attacks[BLACK][source] & (1ULL << enpassant);
                    if (ep_attacks) {
                        target = get_ls1b_index(ep_attacks);
                        int is_check = (piece_bitboards[K] & pawn_attacks[BLACK][target]) ? 1 : 0;
                        moves.add(encode_move(source, target, piece, 0, 1, 0, 1, 0, is_check));
                    }
                }
                
                pop_bit(bb, source);
            }
        }
        
        // Castling
        if (side == WHITE && piece == K) {
            if (castling & WK) {
                if (!get_bit(occupancy[BOTH], f1) && !get_bit(occupancy[BOTH], g1)) {
                    if (!is_square_attacked(e1, BLACK) && !is_square_attacked(f1, BLACK)) {
                        moves.add(encode_move(e1, g1, piece, 0, 0, 0, 0, 1, 0));
                    }
                }
            }
            if (castling & WQ) {
                if (!get_bit(occupancy[BOTH], d1) && !get_bit(occupancy[BOTH], c1) && 
                    !get_bit(occupancy[BOTH], b1)) {
                    if (!is_square_attacked(e1, BLACK) && !is_square_attacked(d1, BLACK)) {
                        moves.add(encode_move(e1, c1, piece, 0, 0, 0, 0, 1, 0));
                    }
                }
            }
        }
        
        if (side == BLACK && piece == k) {
            if (castling & BK) {
                if (!get_bit(occupancy[BOTH], f8) && !get_bit(occupancy[BOTH], g8)) {
                    if (!is_square_attacked(e8, WHITE) && !is_square_attacked(f8, WHITE)) {
                        moves.add(encode_move(e8, g8, piece, 0, 0, 0, 0, 1, 0));
                    }
                }
            }
            if (castling & BQ) {
                if (!get_bit(occupancy[BOTH], d8) && !get_bit(occupancy[BOTH], c8) && 
                    !get_bit(occupancy[BOTH], b8)) {
                    if (!is_square_attacked(e8, WHITE) && !is_square_attacked(d8, WHITE)) {
                        moves.add(encode_move(e8, c8, piece, 0, 0, 0, 0, 1, 0));
                    }
                }
            }
        }
        
        // Knights
        if ((side == WHITE && piece == N) || (side == BLACK && piece == n)) {
            while (bb) {
                source = get_ls1b_index(bb);
                attacks = knight_attacks[source] & ~occupancy[side];
                
                while (attacks) {
                    target = get_ls1b_index(attacks);
                    int is_capture = get_bit(occupancy[side ^ 1], target) ? 1 : 0;
                    int is_check = (piece_bitboards[enemy_king] & knight_attacks[target]) ? 1 : 0;
                    moves.add(encode_move(source, target, piece, 0, is_capture, 0, 0, 0, is_check));
                    pop_bit(attacks, target);
                }
                pop_bit(bb, source);
            }
        }
        
        // Bishops
        if ((side == WHITE && piece == B) || (side == BLACK && piece == b)) {
            while (bb) {
                source = get_ls1b_index(bb);
                attacks = get_bishop_attacks(source, occupancy[BOTH]) & ~occupancy[side];
                
                while (attacks) {
                    target = get_ls1b_index(attacks);
                    int is_capture = get_bit(occupancy[side ^ 1], target) ? 1 : 0;
                    int is_check = (piece_bitboards[enemy_king] & get_bishop_attacks(target, occupancy[BOTH])) ? 1 : 0;
                    moves.add(encode_move(source, target, piece, 0, is_capture, 0, 0, 0, is_check));
                    pop_bit(attacks, target);
                }
                pop_bit(bb, source);
            }
        }
        
        // Rooks
        if ((side == WHITE && piece == R) || (side == BLACK && piece == r)) {
            while (bb) {
                source = get_ls1b_index(bb);
                attacks = get_rook_attacks(source, occupancy[BOTH]) & ~occupancy[side];
                
                while (attacks) {
                    target = get_ls1b_index(attacks);
                    int is_capture = get_bit(occupancy[side ^ 1], target) ? 1 : 0;
                    int is_check = (piece_bitboards[enemy_king] & get_rook_attacks(target, occupancy[BOTH])) ? 1 : 0;
                    moves.add(encode_move(source, target, piece, 0, is_capture, 0, 0, 0, is_check));
                    pop_bit(attacks, target);
                }
                pop_bit(bb, source);
            }
        }
        
        // Queens
        if ((side == WHITE && piece == Q) || (side == BLACK && piece == q)) {
            while (bb) {
                source = get_ls1b_index(bb);
                attacks = get_queen_attacks(source, occupancy[BOTH]) & ~occupancy[side];
                
                while (attacks) {
                    target = get_ls1b_index(attacks);
                    int is_capture = get_bit(occupancy[side ^ 1], target) ? 1 : 0;
                    int is_check = (piece_bitboards[enemy_king] & get_queen_attacks(target, occupancy[BOTH])) ? 1 : 0;
                    moves.add(encode_move(source, target, piece, 0, is_capture, 0, 0, 0, is_check));
                    pop_bit(attacks, target);
                }
                pop_bit(bb, source);
            }
        }
        
        // Kings
        if ((side == WHITE && piece == K) || (side == BLACK && piece == k)) {
            while (bb) {
                source = get_ls1b_index(bb);
                attacks = king_attacks[source] & ~occupancy[side];
                
                while (attacks) {
                    target = get_ls1b_index(attacks);
                    int is_capture = get_bit(occupancy[side ^ 1], target) ? 1 : 0;
                    moves.add(encode_move(source, target, piece, 0, is_capture, 0, 0, 0, 0));
                    pop_bit(attacks, target);
                }
                pop_bit(bb, source);
            }
        }
    }
}

// =============================================================================
// Make Move
// =============================================================================

inline bool Position::make_move(int move, int move_flag) {
    if (move_flag == ALL_MOVES) {
        // Save state for undo
        Position backup;
        copy_to(backup);
        
        int source = get_move_source(move);
        int target = get_move_target(move);
        int piece = get_move_piece(move);
        int promoted = get_move_promoted(move);
        int capture = get_move_capture(move);
        int double_push = get_move_doublepawn(move);
        int ep = get_move_enpassant(move);
        int castle_move = get_move_castling(move);
        
        // Move the piece
        pop_bit(piece_bitboards[piece], source);
        set_bit(piece_bitboards[piece], target);
        
        // Handle capture
        if (capture) {
            int start_piece = (side == WHITE) ? p : P;
            int end_piece = (side == WHITE) ? k : K;
            
            for (int bb_piece = start_piece; bb_piece <= end_piece; bb_piece++) {
                if (get_bit(piece_bitboards[bb_piece], target)) {
                    pop_bit(piece_bitboards[bb_piece], target);
                    break;
                }
            }
        }
        
        // Handle promotion
        if (promoted) {
            pop_bit(piece_bitboards[side == WHITE ? P : p], target);
            set_bit(piece_bitboards[promoted], target);
        }
        
        // Handle en passant capture
        if (ep) {
            if (side == WHITE)
                pop_bit(piece_bitboards[p], target + 8);
            else
                pop_bit(piece_bitboards[P], target - 8);
        }
        
        // Reset en passant square
        enpassant = NO_SQUARE;
        
        // Handle double pawn push
        if (double_push) {
            enpassant = (side == WHITE) ? target + 8 : target - 8;
        }
        
        // Handle castling
        if (castle_move) {
            switch (target) {
                case g1:
                    pop_bit(piece_bitboards[R], h1);
                    set_bit(piece_bitboards[R], f1);
                    break;
                case c1:
                    pop_bit(piece_bitboards[R], a1);
                    set_bit(piece_bitboards[R], d1);
                    break;
                case g8:
                    pop_bit(piece_bitboards[r], h8);
                    set_bit(piece_bitboards[r], f8);
                    break;
                case c8:
                    pop_bit(piece_bitboards[r], a8);
                    set_bit(piece_bitboards[r], d8);
                    break;
            }
        }
        
        // Update castling rights
        castling &= CASTLING_RIGHTS[source];
        castling &= CASTLING_RIGHTS[target];
        
        // Update occupancies
        update_occupancies();
        
        // Switch side
        side ^= 1;
        
        // Check if king is in check (illegal move)
        int king_square = get_ls1b_index(piece_bitboards[side == WHITE ? k : K]);
        if (is_square_attacked(king_square, side)) {
            // Restore state
            backup.copy_to(*this);
            return false;
        }
        
        return true;
    }
    
    // Capture moves only
    if (get_move_capture(move))
        return make_move(move, ALL_MOVES);
    
    return false;
}

// =============================================================================
// FEN Parsing
// =============================================================================

inline void Position::parse_fen(const char* fen) {
    std::memset(piece_bitboards, 0, sizeof(piece_bitboards));
    std::memset(occupancy, 0, sizeof(occupancy));
    side = WHITE;
    enpassant = NO_SQUARE;
    castling = 0;
    
    for (int rank = 0; rank < 8; rank++) {
        for (int file = 0; file < 8; file++) {
            int square = rank * 8 + file;
            
            if ((*fen >= 'a' && *fen <= 'z') || (*fen >= 'A' && *fen <= 'Z')) {
                int piece = char_to_piece(*fen);
                if (piece != NO_PIECE)
                    set_bit(piece_bitboards[piece], square);
                fen++;
            }
            
            if (*fen >= '0' && *fen <= '9') {
                int offset = *fen - '0';
                int found_piece = -1;
                
                for (int p = P; p <= k; p++) {
                    if (get_bit(piece_bitboards[p], square)) {
                        found_piece = p;
                        break;
                    }
                }
                
                if (found_piece == -1) file--;
                file += offset;
                fen++;
            }
            
            if (*fen == '/') fen++;
        }
    }
    
    fen++;
    side = (*fen == 'w') ? WHITE : BLACK;
    fen += 2;
    
    while (*fen != ' ') {
        switch (*fen) {
            case 'K': castling |= WK; break;
            case 'Q': castling |= WQ; break;
            case 'k': castling |= BK; break;
            case 'q': castling |= BQ; break;
        }
        fen++;
    }
    fen++;
    
    if (*fen != '-') {
        int file = fen[0] - 'a';
        int rank = 8 - (fen[1] - '0');
        enpassant = rank * 8 + file;
    }
    
    update_occupancies();
}
