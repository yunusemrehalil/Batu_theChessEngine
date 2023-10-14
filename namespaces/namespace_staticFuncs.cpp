#pragma once
#ifndef STATICFUNCS_CPP
#define STATICFUNCS_CPP
#include "../headers/constants.hpp"
#include "../headers/arrays.hpp"

namespace sif{
    //static inline functions
    static inline int count_bits(U64 bitboard){
        int count = 0;
        while(bitboard)
        {
            count++;
            bitboard &= bitboard -1;
        }
        return count;
    }
    static inline int get_1st_bit_index(U64 bitboard){
        if(bitboard)
        {
            return count_bits((bitboard & -bitboard)-1);
        }
        else return -1;
    }
    static inline U64 get_bishop_attacks(int square, U64 occupancy){
        occupancy &= bishop_masks[square];
        occupancy *= bishop_magic_numbers[square];
        occupancy >>= 64-bishop_relevant_bits[square];
        return bishop_attacks[square][occupancy];
    }
    static inline U64 get_rook_attacks(int square, U64 occupancy){
        occupancy &= rook_masks[square];
        occupancy *= rook_magic_numbers[square];
        occupancy >>= 64-rook_relevant_bits[square];
        return rook_attacks[square][occupancy];
    }
    static inline U64 get_queen_attacks(int square, U64 occupancy){
        U64 queen_attacks = 0ULL;
        U64 bishop_occupancy = occupancy;
        U64 rook_occupancy = occupancy;
        bishop_occupancy &= bishop_masks[square];
        bishop_occupancy *= bishop_magic_numbers[square];
        bishop_occupancy >>= 64 - bishop_relevant_bits[square];
        queen_attacks = bishop_attacks[square][bishop_occupancy];
        rook_occupancy &= rook_masks[square];
        rook_occupancy *= rook_magic_numbers[square];
        rook_occupancy >>= 64 - rook_relevant_bits[square];
        queen_attacks |= rook_attacks[square][rook_occupancy];
        return queen_attacks;
    }  
}
#endif