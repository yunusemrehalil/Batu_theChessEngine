#pragma once

// =============================================================================
// Batu Chess Engine - Types and Definitions
// =============================================================================

#include <cstdint>
#include <cstring>

// =============================================================================
// Basic Types
// =============================================================================

using U64 = unsigned long long;

// =============================================================================
// Board Constants
// =============================================================================

constexpr int BOARD_SIZE = 8;
constexpr int MAX_MOVES = 256;
constexpr int MAX_MOVE_STRING = 6;  // e.g., "e7e8q\0"

// =============================================================================
// Side Constants
// =============================================================================

constexpr int WHITE = 0;
constexpr int BLACK = 1;
constexpr int BOTH = 2;

// =============================================================================
// Search Constants
// =============================================================================

constexpr int INFINITY_SCORE = 999999;
constexpr int CHECKMATE_SCORE = 11111;
constexpr int STALEMATE_SCORE = 0;

// =============================================================================
// Bitboard Masks
// =============================================================================

constexpr U64 NOT_A_FILE = 18374403900871474942ULL;
constexpr U64 NOT_H_FILE = 9187201950435737471ULL;
constexpr U64 NOT_GH_FILE = 4557430888798830399ULL;
constexpr U64 NOT_AB_FILE = 18229723555195321596ULL;

// =============================================================================
// Enumerations
// =============================================================================

enum Square {
    a8, b8, c8, d8, e8, f8, g8, h8,
    a7, b7, c7, d7, e7, f7, g7, h7,
    a6, b6, c6, d6, e6, f6, g6, h6,
    a5, b5, c5, d5, e5, f5, g5, h5,
    a4, b4, c4, d4, e4, f4, g4, h4,
    a3, b3, c3, d3, e3, f3, g3, h3,
    a2, b2, c2, d2, e2, f2, g2, h2,
    a1, b1, c1, d1, e1, f1, g1, h1,
    NO_SQUARE
};

enum Piece { P, R, N, B, Q, K, p, r, n, b, q, k, NO_PIECE };
enum SlidingPiece { ROOK, BISHOP };
enum Castling { WK = 1, WQ = 2, BK = 4, BQ = 8 };
enum MoveType { ALL_MOVES, ONLY_CAPTURES };

// =============================================================================
// Bitboard Macros
// =============================================================================

#define get_bit(bitboard, square) ((bitboard & (1ULL << square)) ? 1 : 0)
#define set_bit(bitboard, square) (bitboard |= (1ULL << square))
#define pop_bit(bitboard, square) ((bitboard) &= ~(1ULL << (square)))

// =============================================================================
// Move Encoding/Decoding
// =============================================================================

/*
    Move encoding (25 bits):
    0000 0000 0000 0000 0011 1111   source square       0x3f
    0000 0000 0000 0000 1111 1100   target square       0xfc0
    0000 0000 0000 1111 0000 0000   piece               0xf000
    0000 0000 1111 0000 0000 0000   promoted piece      0xf0000
    0000 0001 0000 0000 0000 0000   capture flag        0x100000
    0000 0010 0000 0000 0000 0000   double pawn flag    0x200000
    0000 0100 0000 0000 0000 0000   enpassant flag      0x400000
    0000 1000 0000 0000 0000 0000   castling flag       0x800000
    0001 0000 0000 0000 0000 0000   checking flag       0x1000000
*/

#define encode_move(source, target, piece, promoted, capture, doublepawn, enpassant, castling, checking) \
    ((source) | ((target) << 6) | ((piece) << 12) | ((promoted) << 16) | \
     ((capture) << 20) | ((doublepawn) << 21) | ((enpassant) << 22) | \
     ((castling) << 23) | ((checking) << 24))

#define get_move_source(move)    ((move) & 0x3f)
#define get_move_target(move)    (((move) & 0xfc0) >> 6)
#define get_move_piece(move)     (((move) & 0xf000) >> 12)
#define get_move_promoted(move)  (((move) & 0xf0000) >> 16)
#define get_move_capture(move)   ((move) & 0x100000)
#define get_move_doublepawn(move)((move) & 0x200000)
#define get_move_enpassant(move) ((move) & 0x400000)
#define get_move_castling(move)  ((move) & 0x800000)
#define get_move_checking(move)  ((move) & 0x1000000)

// =============================================================================
// Move List Structure
// =============================================================================

struct MoveList {
    int moves[MAX_MOVES];
    int scores[MAX_MOVES];
    int score_guess[MAX_MOVES];
    bool legality[MAX_MOVES];
    int count;
    
    MoveList() : count(0) {}
    
    void add(int move) {
        moves[count] = move;
        count++;
    }
    
    void add_scored(int move, int score) {
        moves[count] = move;
        scores[count] = score;
        count++;
    }
};

// =============================================================================
// Lookup Tables (const data)
// =============================================================================

constexpr int CASTLING_RIGHTS[64] = {
     7, 15, 15, 15,  3, 15, 15, 11,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    13, 15, 15, 15, 12, 15, 15, 14
};

constexpr int BISHOP_RELEVANT_BITS[64] = {
    6, 5, 5, 5, 5, 5, 5, 6,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 7, 7, 7, 7, 5, 5,
    5, 5, 7, 9, 9, 7, 5, 5,
    5, 5, 7, 9, 9, 7, 5, 5,
    5, 5, 7, 7, 7, 7, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    6, 5, 5, 5, 5, 5, 5, 6
};

constexpr int ROOK_RELEVANT_BITS[64] = {
    12, 11, 11, 11, 11, 11, 11, 12,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    12, 11, 11, 11, 11, 11, 11, 12
};

constexpr int BIT_INDEX_64[64] = {
    0,  1, 48,  2, 57, 49, 28,  3,
   61, 58, 50, 42, 38, 29, 17,  4,
   62, 55, 59, 36, 53, 51, 43, 22,
   45, 39, 33, 30, 24, 18, 12,  5,
   63, 47, 56, 27, 60, 41, 37, 16,
   54, 35, 52, 21, 44, 32, 23, 11,
   46, 26, 40, 15, 34, 20, 31, 10,
   25, 14, 19,  9, 13,  8,  7,  6
};

// Piece values for static evaluation (will be replaced by NN)
constexpr int PIECE_VALUES[12] = {
    100,    // P (white pawn)
    500,    // R (white rook)
    300,    // N (white knight)
    320,    // B (white bishop)
    1000,   // Q (white queen)
    10000,  // K (white king)
    -100,   // p (black pawn)
    -500,   // r (black rook)
    -300,   // n (black knight)
    -320,   // b (black bishop)
    -1000,  // q (black queen)
    -10000  // k (black king)
};

// String lookup tables
inline const char* SQUARE_TO_COORD[65] = {
    "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
    "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
    "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
    "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
    "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
    "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
    "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
    "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1", "-"
};

inline const char ASCII_PIECES[12] = {'P','R','N','B','Q','K','p','r','n','b','q','k'};

// Piece char to index mapping
inline int char_to_piece(char c) {
    switch(c) {
        case 'P': return P; case 'R': return R; case 'N': return N;
        case 'B': return B; case 'Q': return Q; case 'K': return K;
        case 'p': return p; case 'r': return r; case 'n': return n;
        case 'b': return b; case 'q': return q; case 'k': return k;
        default: return NO_PIECE;
    }
}

// Promoted piece to char mapping
inline char promoted_to_char(int piece) {
    switch(piece) {
        case Q: case q: return 'q';
        case R: case r: return 'r';
        case B: case b: return 'b';
        case N: case n: return 'n';
        default: return ' ';
    }
}

// =============================================================================
// FEN Positions
// =============================================================================

inline const char* START_POSITION = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
inline const char* KIWIPETE_POSITION = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -";

// Benchmark positions (eval test + mate puzzles)
inline const char* BENCH_EVAL_TEST = "K1B5/P2r4/1p1r1n2/4k3/8/3PPP2/8/8 w - - 0 1";           // SF: -6.0
inline const char* BENCH_MATE_IN_2_A = "1q4b1/8/N2n1NQ1/2P2p2/B1k2rRr/1p1Rp2p/4K3/B7 w - - 0 1";
inline const char* BENCH_MATE_IN_3 = "Q7/8/2K5/8/4N2R/3P4/3Pk3/8 w - - 0 1";
inline const char* BENCH_MATE_IN_2_B = "7B/3B1p2/rP1p2R1/n2k1Pb1/N2Pp3/4P3/K2nN1r1/2R5 w - - 0 1";
