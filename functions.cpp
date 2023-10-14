#include <iostream>
#include <functions.h>
#include <assert.h>
#include <string.h>
#include "headers/magic_numbers.h"
#include "headers/enums.h"
#define U64 unsigned long long
#define BOARD_SIZE 8
#define WHITE 0
#define BLACK 1
#define BOTH 2
using namespace std;


int bitScanForward(U64 bitboard){
   const U64 debruijn64 = 0x03f79d71b4cb0a89;
   assert(bitboard != 0);
   return index64[((bitboard & - bitboard) * debruijn64) >> 58];
}
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
void init_all_pawn_attacks()
{
    for(int i=0; i<BOARD_SIZE*BOARD_SIZE; i++)
    {
        pawn_attacks[WHITE][i] = mask_pawn_attacks(WHITE, i);
        pawn_attacks[BLACK][i] = mask_pawn_attacks(BLACK, i);
    }
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
void init_all_knight_attacks(){
    for(int i=0; i<BOARD_SIZE*BOARD_SIZE; i++)
    {
        knight_attacks[i] = mask_knight_attacks(i);
        knight_attacks[i] = mask_knight_attacks(i);
    }
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
void init_all_king_attacks(){
    for(int i=0; i<BOARD_SIZE*BOARD_SIZE; i++)
    {
        king_attacks[i] = mask_king_attacks(i);
        king_attacks[i] = mask_king_attacks(i);
    }
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
void init_all_leapers_attacks(){
    init_all_pawn_attacks();
    init_all_knight_attacks();
    init_all_king_attacks();
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
void init_all(){
    init_all_leapers_attacks();
    init_all_sliders_attacks(bishop);
    init_all_sliders_attacks(rook);
    //add anothers
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
