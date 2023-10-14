#pragma once
#ifndef MASKPIECEFUNCS_CPP
#define MASKPIECEFUNCS_CPP
#include "../headers/constants.hpp"

namespace maskPieceFuncs{
    U64 mask_pawn_attacks(int side, int square){
        U64 attacks = 0ULL;
        U64 bitboard = 0ULL;
        set_bit(bitboard, square);
        if(!side)
        {
            if((bitboard >> 7) & not_a_file) attacks |= (bitboard >> 7);
            if((bitboard >> 9)& not_h_file) attacks |= (bitboard >> 9);
        }
        else
        {
            if((bitboard << 7) & not_h_file) attacks |= (bitboard << 7);
            if((bitboard << 9)& not_a_file) attacks |= (bitboard << 9);
        }
        return attacks;
    }
    U64 mask_knight_attacks(int square){
        U64 attacks = 0ULL;
        U64 bitboard = 0ULL;
        set_bit(bitboard, square);
        if((bitboard >> 15) & not_a_file) attacks |= (bitboard >> 15);
        if((bitboard >> 17)& not_h_file) attacks |= (bitboard >> 17);
        if((bitboard >> 10)& not_gh_file) attacks |= (bitboard >> 10);
        if((bitboard >> 6)& not_ab_file) attacks |= (bitboard >> 6);
        if((bitboard << 15) & not_h_file) attacks |= (bitboard << 15);
        if((bitboard << 17)& not_a_file) attacks |= (bitboard << 17);
        if((bitboard << 10)& not_ab_file) attacks |= (bitboard << 10);
        if((bitboard << 6)& not_gh_file) attacks |= (bitboard << 6);
        return attacks;
    }
    U64 mask_king_attacks(int square){
        U64 attacks = 0ULL;
        U64 bitboard = 0ULL;
        set_bit(bitboard, square);
        if((bitboard >> 7) & not_a_file) attacks |= (bitboard >> 7);
        if((bitboard >> 9) & not_h_file) attacks |= (bitboard >> 9);
        if((bitboard >> 1) & not_h_file) attacks |= (bitboard >> 1);
        if((bitboard << 1) & not_a_file) attacks |= (bitboard << 1);
        if((bitboard << 7) & not_h_file) attacks |= (bitboard << 7);
        if((bitboard << 9) & not_a_file) attacks |= (bitboard << 9);
        if((bitboard >> 8)) attacks |= (bitboard >> 8);
        if((bitboard << 8)) attacks |= (bitboard << 8);
        return attacks;
    }
    U64 mask_bishop_attacks(int square){
        U64 attacks = 0ULL;
        U64 bitboard = 0ULL;
        set_bit(bitboard, square);
        int i, j, targetI, targetJ;
        targetI = square / 8;
        targetJ = square % 8;
        for(i= targetI+1, j=targetJ+1; i<7 && j<7; i++, j++)
        {
            attacks |= (1ULL<<(i*8+j));
        }
        for(i= targetI-1, j=targetJ+1; i>0 && j<7; i--, j++)
        {
            attacks |= (1ULL<<(i*8+j));
        }
        for(i= targetI+1, j=targetJ-1; i<7 && j>0; i++, j--)
        {
            attacks |= (1ULL<<(i*8+j));
        }
        for(i= targetI-1, j=targetJ-1; i>0 && j>0; i--, j--)
        {
            attacks |= (1ULL<<(i*8+j));
        }
        return attacks;
    }
    U64 mask_rook_attacks(int square){
        U64 attacks = 0ULL;
        U64 bitboard = 0ULL;
        set_bit(bitboard, square);
        int i,j, targetI, targetJ;
        targetI = square/8;
        targetJ = square%8;
        for(i= targetI+1; i<7; i++)
        {
            attacks |= (1ULL<<(i*8)+targetJ);
        }
        for(i= targetI-1; i>0; i--)
        {
            attacks |= (1ULL<<(i*8)+targetJ);
        }
        for(j= targetJ+1; j<7; j++)
        {
            attacks |= (1ULL<<(targetI*8)+j);
        }
        for(j= targetJ-1; j>0; j--)
        {
            attacks |= (1ULL<<(targetI*8)+j);
        }
        return attacks;
    }
    U64 generate_bishop_attacks_with_block(int square, U64 block){
        U64 attacks = 0ULL;
        int i, j, targetI, targetJ;
        targetI = square / 8;
        targetJ = square % 8;
        for(i= targetI+1, j=targetJ+1; i<8 && j<8; i++, j++)
        {
            attacks |= (1ULL<<(i*8+j));
            if((1ULL<<(i*8+j)) & block) break;
        }
        for(i= targetI-1, j=targetJ+1; i>=0 && j<8; i--, j++)
        {
            attacks |= (1ULL<<(i*8+j));
            if((1ULL<<(i*8+j)) & block) break;
            
        }
        for(i= targetI+1, j=targetJ-1; i<8 && j>=0; i++, j--)
        {
            attacks |= (1ULL<<(i*8+j));
            if((1ULL<<(i*8+j)) & block) break;
        }
        for(i= targetI-1, j=targetJ-1; i>=0 && j>=0; i--, j--)
        {
            attacks |= (1ULL<<(i*8+j));
            if((1ULL<<(i*8+j)) & block) break;
        }
        return attacks;
    }
    U64 generate_rook_attacks_with_block(int square, U64 block){
        U64 attacks = 0ULL;
        int i,j, targetI, targetJ;
        targetI = square/8;
        targetJ = square%8;
        for(i= targetI+1; i<8; i++)
        {
            attacks |= (1ULL<<(i*8)+targetJ);
            if((1ULL<<(i*8)+targetJ) & block) break;
        }
        for(i= targetI-1; i>=0; i--)
        {
            attacks |= (1ULL<<(i*8)+targetJ);
            if((1ULL<<(i*8)+targetJ) & block) break;
        }
        for(j= targetJ+1; j<8; j++)
        {
            attacks |= (1ULL<<(targetI*8)+j);
            if((1ULL<<(targetI*8)+j) & block) break;
        }
        for(j= targetJ-1; j>=0; j--)
        {
            attacks |= (1ULL<<(targetI*8)+j);
            if((1ULL<<(targetI*8)+j) & block) break;
        }
        return attacks;
    }

}
#endif