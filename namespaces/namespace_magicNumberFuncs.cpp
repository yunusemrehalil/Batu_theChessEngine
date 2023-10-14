#pragma once
#ifndef MAGICNUMBERFUNCS_CPP
#define MAGICNUMBERFUNCS_CPP

#include "../headers/constants.hpp"
#include "namespace_maskPieceFuncs.cpp"
#include "namespace_initializeFuncs.cpp"
#include "../headers/magic_numbers.hpp"
#include "../headers/enums.hpp"
#include <string.h>
using namespace std;
using namespace maskPieceFuncs;
using namespace initializeFuncs;

namespace magicNumberFuncs {
    unsigned int get_random_number_32(){
        unsigned int number = state;
        number ^= number << 13;
        number ^= number >> 17;
        number ^= number << 5;
        state = number;
        return number;
    }
    U64 get_random_number_64(){
        U64 r1, r2, r3, r4;
        r1 = (U64)(get_random_number_32() & 0xFFFF);
        r2 = (U64)(get_random_number_32() & 0xFFFF);
        r3 = (U64)(get_random_number_32() & 0xFFFF);
        r4 = (U64)(get_random_number_32() & 0xFFFF);
        return r1 | (r2<<16) | (r3<<32) | (r4<<48);
    }
    U64 generate_magic_number(){
        return (get_random_number_64() & get_random_number_64() & get_random_number_64());
    }
    U64 find_magic_number(int square, int relevant_bits, int bishop){
        U64 occupancies[4096];
        U64 attacks[4096];
        U64 used_attacks[4096];
        U64 attack_mask = bishop ? mask_bishop_attacks(square): mask_rook_attacks(square);
        int occupancy_indices = 1 << relevant_bits;
        for(int i=0; i<occupancy_indices; i++)
        {
            occupancies[i] = set_occupancy(i, relevant_bits, attack_mask);
            attacks[i] = bishop ? generate_bishop_attacks_with_block(square, occupancies[i]): generate_rook_attacks_with_block(square, occupancies[i]);
        } 
        for(int random=0; random<100000000; random++)
        {
            U64 magic_number = generate_magic_number();
            if (count_bits((attack_mask*magic_number) & 0xFF00000000000000)<6) continue; 
            memset(used_attacks, 0ULL, sizeof(used_attacks));
            int i, j;
            for(i=0, j=0; !j && i < occupancy_indices; i++)
            {
                int magic_index = (int)((occupancies[i] * magic_number) >> (64-relevant_bits));
                if(used_attacks[magic_index] == 0ULL)
                {
                    used_attacks[magic_index] = attacks[i];
                }
                else if(used_attacks[magic_index] != attacks[i])
                {
                    j=1;
                }
            }
            if(!j) return magic_number;
        }
        cout<<"failed";
        return 0ULL;
    }
    void init_magic_numbers(){
        for(int i=0; i<BOARD_SIZE*BOARD_SIZE; i++)
        {
            rook_magic_numbers[i] = find_magic_number(i, rook_relevant_bits[i], rook);
        }
        for(int i=0; i<BOARD_SIZE*BOARD_SIZE; i++)
        {
            bishop_magic_numbers[i] = find_magic_number(i, bishop_relevant_bits[i], bishop);
        }
    }   
    }

#endif