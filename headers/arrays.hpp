#pragma once
#ifndef ARRAYS_H
#define ARRAYS_H

#include "constants.hpp"
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



#endif
