#pragma once
#ifndef INITIALIZEFUNCS_CPP
#define INITIALIZEFUNCS_CPP
#include "../headers/constants.hpp"
#include "../headers/arrays.hpp"
#include "namespace_maskPieceFuncs.cpp"
#include "namespace_staticFuncs.cpp"

using namespace maskPieceFuncs;
using namespace sif;

namespace initializeFuncs{
    U64 set_occupancy(int index, int bits_in_mask, U64 attack_mask){
        U64 occupancy = 0ULL;
        for(int i=0; i<bits_in_mask; i++)
        {
            int square = get_1st_bit_index(attack_mask);
            delete_bit(attack_mask, square);
            if(index &  (1<<i))
            {
                occupancy |= (1ULL << square);
            }
        }
        return occupancy;
    }
    void init_all_pawn_attacks()
    {
        for(int i=0; i<BOARD_SIZE*BOARD_SIZE; i++)
        {
            pawn_attacks[WHITE][i] = mask_pawn_attacks(WHITE, i);
            pawn_attacks[BLACK][i] = mask_pawn_attacks(BLACK, i);
        }
    }
    void init_all_knight_attacks(){
        for(int i=0; i<BOARD_SIZE*BOARD_SIZE; i++)
        {
            knight_attacks[i] = mask_knight_attacks(i);
            knight_attacks[i] = mask_knight_attacks(i);
        }
    }
    void init_all_king_attacks(){
        for(int i=0; i<BOARD_SIZE*BOARD_SIZE; i++)
        {
            king_attacks[i] = mask_king_attacks(i);
            king_attacks[i] = mask_king_attacks(i);
        }
    }
    void init_all_leapers_attacks(){
        init_all_pawn_attacks();
        init_all_knight_attacks();
        init_all_king_attacks();
    }
    void init_all_sliders_attacks(int bishop){
        for(int i=0; i<BOARD_SIZE*BOARD_SIZE; i++)
        {
            bishop_masks[i] = mask_bishop_attacks(i);
            rook_masks[i] = mask_rook_attacks(i);
            U64 attack_mask = bishop ? bishop_masks[i] : rook_masks[i];
            int relevant_bits_count = count_bits(attack_mask);
            int occupancy_indices = (1 << relevant_bits_count);
            for(int j=0; j<occupancy_indices; j++)
            {
                if(bishop)
                {
                    U64 occupancy = set_occupancy(j, relevant_bits_count, attack_mask);
                    int magic_index = (occupancy * bishop_magic_numbers[i]) >> (64-bishop_relevant_bits[i]);
                    bishop_attacks[i][magic_index] = generate_bishop_attacks_with_block(i, occupancy);
                }
                else
                {
                    U64 occupancy = set_occupancy(j, relevant_bits_count, attack_mask);
                    int magic_index = (occupancy * rook_magic_numbers[i]) >> (64-rook_relevant_bits[i]);
                    rook_attacks[i][magic_index] = generate_rook_attacks_with_block(i, occupancy);
                }
            }
        }
    }
    void init_all(){
        init_all_leapers_attacks();
        init_all_sliders_attacks(bishop);
        init_all_sliders_attacks(rook);
        //add anothers
    }
}
#endif