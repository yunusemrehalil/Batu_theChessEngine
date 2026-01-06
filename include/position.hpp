#pragma once

// =============================================================================
// Batu Chess Engine - Position (Game State)
// =============================================================================

#include "types.hpp"
#include "magic_numbers.hpp"
#include <cstring>
#include <iostream>

// =============================================================================
// Attack Tables (Global - Initialized Once)
// =============================================================================

inline U64 pawn_attacks[2][64];
inline U64 knight_attacks[64];
inline U64 king_attacks[64];
inline U64 bishop_masks[64];
inline U64 rook_masks[64];
inline U64 bishop_attacks[64][512];
inline U64 rook_attacks[64][4096];

// =============================================================================
// Position Class - Encapsulates All Game State
// =============================================================================

class Position {
public:
    // Bitboards
    U64 piece_bitboards[12];
    U64 occupancy[3];  // WHITE, BLACK, BOTH
    
    // Game state
    int side;
    int enpassant;
    int castling;
    
    // Search statistics
    long nodes;
    
    // ==========================================================================
    // Constructors
    // ==========================================================================
    
    Position() : side(WHITE), enpassant(NO_SQUARE), castling(0), nodes(0) {
        std::memset(piece_bitboards, 0, sizeof(piece_bitboards));
        std::memset(occupancy, 0, sizeof(occupancy));
    }
    
    // ==========================================================================
    // Board State Management
    // ==========================================================================
    
    void copy_to(Position& dest) const {
        std::memcpy(dest.piece_bitboards, piece_bitboards, sizeof(piece_bitboards));
        std::memcpy(dest.occupancy, occupancy, sizeof(occupancy));
        dest.side = side;
        dest.enpassant = enpassant;
        dest.castling = castling;
    }
    
    void update_occupancies() {
        occupancy[WHITE] = 0ULL;
        occupancy[BLACK] = 0ULL;
        
        for (int piece = P; piece <= K; piece++)
            occupancy[WHITE] |= piece_bitboards[piece];
        for (int piece = p; piece <= k; piece++)
            occupancy[BLACK] |= piece_bitboards[piece];
        
        occupancy[BOTH] = occupancy[WHITE] | occupancy[BLACK];
    }
    
    // ==========================================================================
    // Bitboard Utilities
    // ==========================================================================
    
    static int count_bits(U64 bitboard) {
        int count = 0;
        while (bitboard) {
            count++;
            bitboard &= bitboard - 1;
        }
        return count;
    }
    
    static int get_ls1b_index(U64 bitboard) {
        if (bitboard)
            return count_bits((bitboard & -bitboard) - 1);
        return -1;
    }
    
    // ==========================================================================
    // Attack Generation
    // ==========================================================================
    
    static U64 get_bishop_attacks(int square, U64 occ) {
        occ &= bishop_masks[square];
        occ *= BISHOP_MAGIC_NUMBERS[square];
        occ >>= 64 - BISHOP_RELEVANT_BITS[square];
        return bishop_attacks[square][occ];
    }
    
    static U64 get_rook_attacks(int square, U64 occ) {
        occ &= rook_masks[square];
        occ *= ROOK_MAGIC_NUMBERS[square];
        occ >>= 64 - ROOK_RELEVANT_BITS[square];
        return rook_attacks[square][occ];
    }
    
    static U64 get_queen_attacks(int square, U64 occ) {
        return get_bishop_attacks(square, occ) | get_rook_attacks(square, occ);
    }
    
    bool is_square_attacked(int square, int attacking_side) const {
        // Pawn attacks
        if (attacking_side == WHITE) {
            if (pawn_attacks[BLACK][square] & piece_bitboards[P]) return true;
        } else {
            if (pawn_attacks[WHITE][square] & piece_bitboards[p]) return true;
        }
        
        // Knight attacks
        if (knight_attacks[square] & piece_bitboards[attacking_side == WHITE ? N : n])
            return true;
        
        // King attacks
        if (king_attacks[square] & piece_bitboards[attacking_side == WHITE ? K : k])
            return true;
        
        // Bishop/Queen attacks
        U64 bishops_queens = piece_bitboards[attacking_side == WHITE ? B : b] |
                            piece_bitboards[attacking_side == WHITE ? Q : q];
        if (get_bishop_attacks(square, occupancy[BOTH]) & bishops_queens)
            return true;
        
        // Rook/Queen attacks
        U64 rooks_queens = piece_bitboards[attacking_side == WHITE ? R : r] |
                          piece_bitboards[attacking_side == WHITE ? Q : q];
        if (get_rook_attacks(square, occupancy[BOTH]) & rooks_queens)
            return true;
        
        return false;
    }
    
    // ==========================================================================
    // Move Generation
    // ==========================================================================
    
    void generate_moves(MoveList& moves) const;
    
    // ==========================================================================
    // Make Move
    // ==========================================================================
    
    bool make_move(int move, int move_flag);
    
    // ==========================================================================
    // FEN Parsing
    // ==========================================================================
    
    void parse_fen(const char* fen);
    
    // ==========================================================================
    // Evaluation (Static - Will be replaced by NN)
    // ==========================================================================
    
    int evaluate() const {
        int score = 0;
        
        for (int piece = P; piece <= k; piece++) {
            U64 bb = piece_bitboards[piece];
            while (bb) {
                get_ls1b_index(bb);
                score += PIECE_VALUES[piece];
                bb &= bb - 1;
            }
        }
        
        return (side == WHITE) ? score : -score;
    }
    
    // ==========================================================================
    // Print Functions
    // ==========================================================================
    
    void print() const {
        std::cout << std::endl;
        for (int rank = 0; rank < 8; rank++) {
            std::cout << " " << (8 - rank) << " ";
            for (int file = 0; file < 8; file++) {
                int square = rank * 8 + file;
                int piece = -1;
                
                for (int p = P; p <= k; p++) {
                    if (get_bit(piece_bitboards[p], square)) {
                        piece = p;
                        break;
                    }
                }
                
                std::cout << " " << (piece == -1 ? '.' : ASCII_PIECES[piece]) << " ";
            }
            std::cout << std::endl;
        }
        std::cout << "    a  b  c  d  e  f  g  h" << std::endl << std::endl;
        std::cout << " Side: " << (side == WHITE ? "white" : "black") << std::endl;
        std::cout << " Enpassant: " << (enpassant != NO_SQUARE ? SQUARE_TO_COORD[enpassant] : "no") << std::endl;
        std::cout << " Castling: " 
                  << ((castling & WK) ? 'K' : '-')
                  << ((castling & WQ) ? 'Q' : '-')
                  << ((castling & BK) ? 'k' : '-')
                  << ((castling & BQ) ? 'q' : '-') << std::endl;
    }
    
    static void print_move(int move) {
        std::cout << SQUARE_TO_COORD[get_move_source(move)]
                  << SQUARE_TO_COORD[get_move_target(move)]
                  << promoted_to_char(get_move_promoted(move));
    }
};

// =============================================================================
// Global Position Instance (for backward compatibility during refactoring)
// =============================================================================

inline Position g_pos;
