#pragma once
#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <vector>
#include <string>

#define BOARD_SIZE 8
#define WHITE 0
#define BLACK 1
#define BOTH 2
#define NEGATIVEINFINITY -999999
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

#define encode_move(source, target, piece, promoted, capture, doublepawn, enpassant, castling, checking)\
        (source) |          \
        (target << 6) |     \
        (piece << 12) |     \
        (promoted << 16) |     \
        (capture << 20) |     \
        (doublepawn << 21) |     \
        (enpassant << 22) |     \
        (castling << 23) |     \
        (checking << 24)     \
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
//extract check
#define get_move_checking(move) (move & 0x1000000)
const char* empty_board = "8/8/8/8/8/8/8/8 b - - ";
const char* start_position = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
const char* tricky_position = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - ";
const char* killer_position = "rnbqkb1r/pp1p1ppp/8/2p1pP2/1P1P4/3P3P/P1P1P3/RNBQKBNR w KQkq e6 0 1";
const char* cmk_position = "r2q1rk1/ppp2ppp/2n1bn2/2b1p3/3pP3/3P1NPP/PPP1NPB1/R1BQ1RK1 b - - 0 9";
const char* random_position = "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1";
const char* after_starting_moves_position = "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8 ";
const char* middle_game = "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10 ";
const char* programming_1 = "3k4/3p4/8/K1P4r/8/8/8/8 b - - 0 1";
const char* programming_2 = "8/8/4k3/8/2p5/8/B2P2K1/8 w - - 0 1";
const char* programming_3 = "8/8/1k6/2b5/2pP4/8/5K2/8 b - d3 0 1";
const char* programming_4 = "5k2/8/8/8/8/8/8/4K2R w K - 0 1";
const char* programming_5 = "3k4/8/8/8/8/8/8/R3K3 w Q - 0 1";
const char* programming_6 = "r3k2r/1b4bq/8/8/8/8/7B/R3K2R w KQkq - 0 1";
const char* programming_7 = "r3k2r/8/3Q4/8/8/5q2/8/R3K2R b KQkq - 0 1";
const char* programming_8 = "2K2r2/4P3/8/8/8/8/8/3k4 w - - 0 1";
const char* programming_9 = "8/8/1P2K3/8/2n5/1q6/8/5k2 b - - 0 1";
const char* programming_10 = "4k3/1P6/8/8/8/8/K7/8 w - - 0 1";
const char* programming_11 = "8/P1k5/K7/8/8/8/8/8 w - - 0 1";
const char* programming_12 = "K1k5/8/P7/8/8/8/8/8 w - - 0 1";
const char* programming_13 = "8/k1P5/8/1K6/8/8/8/8 w - - 0 1";
const char* programming_14 = "8/8/2k5/5q2/5n2/8/5K2/8 b - - 0 1";
const char* wrong_position = "1N6/6k1/8/8/7B/8/8/4K3 w - - 19 103";
const char* trying_position = "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1";
const char* my_position = "8/8/4k3/8/2pP4/8/B5K1/8 b - d3 0 1";
const char* my_Second_position = "8/8/4k3/8/2p5/8/B2P2K1/8 w - - 0 1";
const char* my_third_position = "8/2p5/3p4/KP5r/1R2Pp1k/8/6P1/8 b - e3 0 1";
const char* end_game = "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - ";
const char* end_game_2 = "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1";
const char* end_game_3 = "8/2p5/3p4/KP6/R1r2pPk/4P3/8/8 b - g3 0 3";
const char* end_game_4 = "1N6/6k1/8/8/7B/8/8/4K3 w - - 0 1";
const char* end_game_5 = "8/2p5/3p4/KP5r/1R3pPk/8/4P3/8 b - g3 0 1";
const char* end_game_6 = "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1";
const char* end_game_7 = "8/2p5/3p4/KP6/R1r2pPk/4P3/8/8 b - g3 0 3";
const char* white_knight_can_check_position = "r1q3k1/1Pp2N2/2N1b1pr/4Pp1p/2Pp2nP/3P1PP1/p5B1/1NBQ1RK1 w - f6 0 25";
const char* white_bishop_and_queen_can_check_position = "rnbqkbnr/ppp1p1pp/5p2/3p4/4P3/P7/1PPP1PPP/RNBQKBNR w KQkq - 0 3";
const char* both_side_check_position = "3k2r1/5P2/2b1Pb1r/2N5/R7/1Qpn4/1q2B1p1/3KBN2 w - - 0 1";
const char* pawn_2_ahead_check_position = "8/6k1/5n2/4ppPp/3N1B2/3P2K1/P7/8 w - h6 0 1";
const char* black_promotion_position_with_enpassant_and_captures = "r1q2rk1/1Ppp4/2N1b1p1/4Pp1p/1nPp2nP/3P1PP1/p5B1/1NBQ1RK1 b - c3 0 25";
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
const int castling_rights[64] = {
     7, 15, 15, 15,  3, 15, 15, 11,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    13, 15, 15, 15, 12, 15, 15, 14
};
char ascii_pieces[12] = {'P','R','N','B','Q','K','p','r','n','b','q','k'};
unsigned int state = 1804289383;
int pieceValue[12] = {
    100,
    500,
    300,
    320,
    1000,
    10000,
    -100,
    -500,
    -300,
    -320,
    -1000,
    -10000
};


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