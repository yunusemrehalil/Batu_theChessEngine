#pragma once
#ifndef ENUMS_H
#define ENUMS_H
#include <unordered_map>

using namespace std;

enum sliding_pieces{rook, bishop};
enum squares{
a8, b8, c8, d8, e8, f8, g8, h8,
a7, b7, c7, d7, e7, f7, g7, h7,
a6, b6, c6, d6, e6, f6, g6, h6,
a5, b5, c5, d5, e5, f5, g5, h5,
a4, b4, c4, d4, e4, f4, g4, h4,
a3, b3, c3, d3, e3, f3, g3, h3,
a2, b2, c2, d2, e2, f2, g2, h2,
a1, b1, c1, d1, e1, f1, g1, h1, no_sq
};
enum castling{wk=1, wq=2, bk=4, bq=8};
enum pieces{P, R, N, B, Q, K, p, r, n, b, q, k};
enum move_types{all_moves, only_captures};

unordered_map<char, int> char_pieces = {
    {'P', P}, {'R', R}, {'N', N}, {'B', B}, {'Q', Q}, {'K', K},
    {'p', p}, {'r', r}, {'n', n}, {'b', b}, {'q', q}, {'k', k}
};
unordered_map<int, char> promoted_piece = {
    {Q, 'q'}, {R, 'r'}, {B, 'b'}, {N, 'n'}, {q, 'q'}, {r, 'r'},
    {b, 'b'}, {n, 'n'} 
};


#endif
