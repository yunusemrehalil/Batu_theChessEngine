#pragma once
#include <assert.h>
#include "constants.h"
#define U64 unsigned long long
//arrays
U64 pawn_attacks[2][64];
U64 knight_attacks[64];
U64 king_attacks[64];
U64 bishop_masks[64];
U64 rook_masks[64];
U64 bishop_attacks[64][512];
U64 rook_attacks[64][4096];
U64 piece_bitboards[12];
U64 occupancy_bitboards[3];

//functions
int bitScanForward(U64 bitboard);
U64 mask_pawn_attacks(int side, int square);
U64 mask_knight_attacks(int square);
U64 mask_king_attacks(int square);
U64 mask_bishop_attacks(int square);
U64 mask_rook_attacks(int square);
U64 generate_bishop_attacks_with_block(int square, U64 block);
U64 generate_rook_attacks_with_block(int square, U64 block);
void init_all_leapers_attacks();
void init_all_sliders_attacks(int bishop);
void init_all_pawn_attacks();
void init_all_knight_attacks();
void init_all_king_attacks();
void init_magic_numbers();
void init_all();
U64 set_occupancy(int index, int bits_in_mask, U64 attack_mask);
unsigned int state = 1804289383;
unsigned int get_random_number_32();
U64 get_random_number_64();
U64 generate_magic_number();
U64 find_magic_number(int square, int relevant_bits, U64 bishop);

//static inline functions
static inline int count_bits(U64 bitboard);
static inline int get_1st_bit_index(U64 bitboard);
static inline U64 get_bishop_attacks(int square, U64 occupancy);
static inline U64 get_rook_attacks(int square, U64 occupancy);

