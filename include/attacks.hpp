#pragma once

// =============================================================================
// Batu Chess Engine - Attack Table Initialization
// =============================================================================

#include "position.hpp"

namespace AttackTables {

// =============================================================================
// Mask Functions (for magic bitboard generation)
// =============================================================================

inline U64 mask_pawn_attacks(int side, int square) {
    U64 attacks = 0ULL;
    U64 bb = 0ULL;
    set_bit(bb, square);
    
    if (side == WHITE) {
        if ((bb >> 7) & NOT_A_FILE) attacks |= (bb >> 7);
        if ((bb >> 9) & NOT_H_FILE) attacks |= (bb >> 9);
    } else {
        if ((bb << 7) & NOT_H_FILE) attacks |= (bb << 7);
        if ((bb << 9) & NOT_A_FILE) attacks |= (bb << 9);
    }
    return attacks;
}

inline U64 mask_knight_attacks(int square) {
    U64 attacks = 0ULL;
    U64 bb = 0ULL;
    set_bit(bb, square);
    
    if ((bb >> 15) & NOT_A_FILE)  attacks |= (bb >> 15);
    if ((bb >> 17) & NOT_H_FILE)  attacks |= (bb >> 17);
    if ((bb >> 10) & NOT_GH_FILE) attacks |= (bb >> 10);
    if ((bb >> 6)  & NOT_AB_FILE) attacks |= (bb >> 6);
    if ((bb << 15) & NOT_H_FILE)  attacks |= (bb << 15);
    if ((bb << 17) & NOT_A_FILE)  attacks |= (bb << 17);
    if ((bb << 10) & NOT_AB_FILE) attacks |= (bb << 10);
    if ((bb << 6)  & NOT_GH_FILE) attacks |= (bb << 6);
    
    return attacks;
}

inline U64 mask_king_attacks(int square) {
    U64 attacks = 0ULL;
    U64 bb = 0ULL;
    set_bit(bb, square);
    
    if ((bb >> 7) & NOT_A_FILE) attacks |= (bb >> 7);
    if ((bb >> 9) & NOT_H_FILE) attacks |= (bb >> 9);
    if ((bb >> 1) & NOT_H_FILE) attacks |= (bb >> 1);
    if ((bb << 1) & NOT_A_FILE) attacks |= (bb << 1);
    if ((bb << 7) & NOT_H_FILE) attacks |= (bb << 7);
    if ((bb << 9) & NOT_A_FILE) attacks |= (bb << 9);
    if (bb >> 8) attacks |= (bb >> 8);
    if (bb << 8) attacks |= (bb << 8);
    
    return attacks;
}

inline U64 mask_bishop_attacks(int square) {
    U64 attacks = 0ULL;
    int r, f;
    int tr = square / 8;
    int tf = square % 8;
    
    for (r = tr + 1, f = tf + 1; r < 7 && f < 7; r++, f++)
        attacks |= (1ULL << (r * 8 + f));
    for (r = tr - 1, f = tf + 1; r > 0 && f < 7; r--, f++)
        attacks |= (1ULL << (r * 8 + f));
    for (r = tr + 1, f = tf - 1; r < 7 && f > 0; r++, f--)
        attacks |= (1ULL << (r * 8 + f));
    for (r = tr - 1, f = tf - 1; r > 0 && f > 0; r--, f--)
        attacks |= (1ULL << (r * 8 + f));
    
    return attacks;
}

inline U64 mask_rook_attacks(int square) {
    U64 attacks = 0ULL;
    int r, f;
    int tr = square / 8;
    int tf = square % 8;
    
    for (r = tr + 1; r < 7; r++)
        attacks |= (1ULL << (r * 8 + tf));
    for (r = tr - 1; r > 0; r--)
        attacks |= (1ULL << (r * 8 + tf));
    for (f = tf + 1; f < 7; f++)
        attacks |= (1ULL << (tr * 8 + f));
    for (f = tf - 1; f > 0; f--)
        attacks |= (1ULL << (tr * 8 + f));
    
    return attacks;
}

inline U64 bishop_attacks_on_the_fly(int square, U64 block) {
    U64 attacks = 0ULL;
    int r, f;
    int tr = square / 8;
    int tf = square % 8;
    
    for (r = tr + 1, f = tf + 1; r < 8 && f < 8; r++, f++) {
        attacks |= (1ULL << (r * 8 + f));
        if ((1ULL << (r * 8 + f)) & block) break;
    }
    for (r = tr - 1, f = tf + 1; r >= 0 && f < 8; r--, f++) {
        attacks |= (1ULL << (r * 8 + f));
        if ((1ULL << (r * 8 + f)) & block) break;
    }
    for (r = tr + 1, f = tf - 1; r < 8 && f >= 0; r++, f--) {
        attacks |= (1ULL << (r * 8 + f));
        if ((1ULL << (r * 8 + f)) & block) break;
    }
    for (r = tr - 1, f = tf - 1; r >= 0 && f >= 0; r--, f--) {
        attacks |= (1ULL << (r * 8 + f));
        if ((1ULL << (r * 8 + f)) & block) break;
    }
    
    return attacks;
}

inline U64 rook_attacks_on_the_fly(int square, U64 block) {
    U64 attacks = 0ULL;
    int r, f;
    int tr = square / 8;
    int tf = square % 8;
    
    for (r = tr + 1; r < 8; r++) {
        attacks |= (1ULL << (r * 8 + tf));
        if ((1ULL << (r * 8 + tf)) & block) break;
    }
    for (r = tr - 1; r >= 0; r--) {
        attacks |= (1ULL << (r * 8 + tf));
        if ((1ULL << (r * 8 + tf)) & block) break;
    }
    for (f = tf + 1; f < 8; f++) {
        attacks |= (1ULL << (tr * 8 + f));
        if ((1ULL << (tr * 8 + f)) & block) break;
    }
    for (f = tf - 1; f >= 0; f--) {
        attacks |= (1ULL << (tr * 8 + f));
        if ((1ULL << (tr * 8 + f)) & block) break;
    }
    
    return attacks;
}

inline U64 set_occupancy(int index, int bits_in_mask, U64 attack_mask) {
    U64 occupancy = 0ULL;
    
    for (int count = 0; count < bits_in_mask; count++) {
        int square = Position::get_ls1b_index(attack_mask);
        pop_bit(attack_mask, square);
        
        if (index & (1 << count))
            occupancy |= (1ULL << square);
    }
    
    return occupancy;
}

// =============================================================================
// Initialization Functions
// =============================================================================

inline void init_leaper_attacks() {
    for (int square = 0; square < 64; square++) {
        pawn_attacks[WHITE][square] = mask_pawn_attacks(WHITE, square);
        pawn_attacks[BLACK][square] = mask_pawn_attacks(BLACK, square);
        knight_attacks[square] = mask_knight_attacks(square);
        king_attacks[square] = mask_king_attacks(square);
    }
}

inline void init_slider_attacks(int is_bishop) {
    for (int square = 0; square < 64; square++) {
        bishop_masks[square] = mask_bishop_attacks(square);
        rook_masks[square] = mask_rook_attacks(square);
        
        U64 attack_mask = is_bishop ? bishop_masks[square] : rook_masks[square];
        int relevant_bits = Position::count_bits(attack_mask);
        int occupancy_count = (1 << relevant_bits);
        
        for (int index = 0; index < occupancy_count; index++) {
            if (is_bishop) {
                U64 occ = set_occupancy(index, relevant_bits, attack_mask);
                int magic_index = (occ * BISHOP_MAGIC_NUMBERS[square]) >> 
                                  (64 - BISHOP_RELEVANT_BITS[square]);
                bishop_attacks[square][magic_index] = bishop_attacks_on_the_fly(square, occ);
            } else {
                U64 occ = set_occupancy(index, relevant_bits, attack_mask);
                int magic_index = (occ * ROOK_MAGIC_NUMBERS[square]) >> 
                                  (64 - ROOK_RELEVANT_BITS[square]);
                rook_attacks[square][magic_index] = rook_attacks_on_the_fly(square, occ);
            }
        }
    }
}

inline void init_all() {
    init_leaper_attacks();
    init_slider_attacks(BISHOP);
    init_slider_attacks(ROOK);
}

} // namespace AttackTables
