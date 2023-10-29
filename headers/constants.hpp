#pragma once
#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <vector>
#include <string>

#define BOARD_SIZE 8
#define WHITE 0
#define BLACK 1
#define BOTH 2
#define U64 unsigned long long
#define get_bit(bitboard, square) ((bitboard & (1ULL << square))?1:0)
#define set_bit(bitboard, square) (bitboard |= (1ULL << square))
#define delete_bit(bitboard, square) ((bitboard) &= ~(1ULL << (square)))
#define copy_board()                                                                \
        U64 bitboards_copy[12], occupancies_copy[3];                                \
        int side_copy, enpassant_copy, castling_copy;                               \
        memcpy(bitboards_copy, piece_bitboards, sizeof(piece_bitboards));           \
        memcpy(occupancies_copy, occupancy_bitboards, sizeof(occupancy_bitboards)); \
        side_copy = side; enpassant_copy = enpassant; castling_copy = castle;       \
        
#define take_back()                                                                 \
        memcpy(piece_bitboards, bitboards_copy, sizeof(bitboards_copy));            \
        memcpy(occupancy_bitboards, occupancies_copy, sizeof(occupancies_copy));    \
        side = side_copy; enpassant = enpassant_copy; castle = castling_copy;       \

#define encode_move(source, target, piece, promoted, capture, doublepawn, enpassant, castling)\
        (source) |          \
        (target << 6) |     \
        (piece << 12) |     \
        (promoted << 16) |     \
        (capture << 20) |     \
        (doublepawn << 21) |     \
        (enpassant << 22) |     \
        (castling << 23)      \
//extract source square
#define get_move_source(move) (move & 0x3f)
//extract target square
#define get_move_target(move) ((move & 0xfc0)>>6)
//extract piece
#define get_move_piece(move) ((move & 0xf000)>>12)
//extract promoted
#define get_move_promoted(move) ((move & 0xf0000)>>16)
//extract capture
#define get_move_capture(move) (move & 0x100000)
//extract doublepawn
#define get_move_doublepawn(move) (move & 0x200000)
//extract enpassant
#define get_move_enpassant(move) (move & 0x400000)
//extract doublepawn
#define get_move_castling(move) (move & 0x800000)
const char* empty_board = "8/8/8/8/8/8/8/8 b - - ";
const char* start_position = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
const char* tricky_position = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
const char* killer_position = "rnbqkb1r/pp1p1ppp/8/2p1pP2/1P1P4/3P3P/P1P1P3/RNBQKBNR w KQkq e6 0 1";
const char* cmk_position = "r2q1rk1/ppp2ppp/2n1bn2/2b1p3/3pP3/3P1NPP/PPP1NPB1/R1BQ1RK1 b - - 0 9";
const char* white_knight_can_check_position = "r1q3k1/1Pp2N2/2N1b1pr/4Pp1p/2Pp2nP/3P1PP1/p5B1/1NBQ1RK1 w - f6 0 25";
const char* white_bishop_and_queen_can_check_position = "rnbqkbnr/ppp1p1pp/5p2/3p4/4P3/P7/1PPP1PPP/RNBQKBNR w KQkq - 0 3";
const char* both_side_check_position = "3k2r1/5P2/2b1Pb1r/2N5/R7/1Qpn4/1q2B1p1/3KBN2 w - - 0 1";
const char* pawn_2_ahead_check_position = "8/6k1/5n2/4ppPp/3N1B2/3P2K1/P7/8 w - h6 0 1";
const char* black_promotion_position_with_enpassant_and_captures = "r1q2rk1/1Ppp4/2N1b1p1/4Pp1p/2Pp2nP/2nP1PP1/p5B1/1NBQ1RK1 b - - 0 25";
const char* white_promotion_position_with_enpassant_and_captures = "r1q2rk1/1Pp5/2N1b1p1/4Pp1p/2Pp2nP/3P1PP1/p5B1/1NBQ1RK1 w - f6 0 25";
const char* castling_position = "r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1";
const char* castling_trying = "r3k2r/8/4B3/8/8/7n/8/R3K2R b KQkq - 0 1";
const U64 not_a_file = 18374403900871474942ULL;
const U64 not_h_file = 9187201950435737471ULL;
const U64 not_gh_file = 4557430888798830399ULL;
const U64 not_ab_file = 18229723555195321596ULL;
const int index64[64]= {
    0,  1, 48,  2, 57, 49, 28,  3,
   61, 58, 50, 42, 38, 29, 17,  4,
   62, 55, 59, 36, 53, 51, 43, 22,
   45, 39, 33, 30, 24, 18, 12,  5,
   63, 47, 56, 27, 60, 41, 37, 16,
   54, 35, 52, 21, 44, 32, 23, 11,
   46, 26, 40, 15, 34, 20, 31, 10,
   25, 14, 19,  9, 13,  8,  7,  6
};
const int bishop_relevant_bits[64] = {
    6, 5, 5, 5, 5, 5, 5, 6,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 7, 7, 7, 7, 5, 5,
    5, 5, 7, 9, 9, 7, 5, 5,
    5, 5, 7, 9, 9, 7, 5, 5,
    5, 5, 7, 7, 7, 7, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    6, 5, 5, 5, 5, 5, 5, 6
};
const int rook_relevant_bits[64] = {
    12, 11, 11, 11, 11, 11, 11, 12,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    12, 11, 11, 11, 11, 11, 11, 12
};
const char *square_to_coordinate[] = {
"a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
"a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
"a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
"a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
"a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
"a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
"a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
"a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1"
};
const char white_promotions[] = {'Q', 'R', 'B', 'N'};
const char black_promotions[] = {'q', 'r', 'b', 'n'};
char ascii_pieces[12] = {'P','R','N','B','Q','K','p','r','n','b','q','k'};
unsigned int state = 1804289383;
std::string checks[64];
std::string center_captures[64];
std::string promote_captures[64];
std::string captures[64];
std::string centers[64];
std::string promotion_moves[64];
std::string others[64];
typedef struct 
{
    int moves[256];
    int count;
}moves;
#endif