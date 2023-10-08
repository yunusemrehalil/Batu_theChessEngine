#include <iostream>
#define U64 unsigned long long
#define BOARD_SIZE 8
#define WHITE 0
#define BLACK 1
#define get_bit(bitboard, square) ((bitboard & (1ULL << square))?1:0)
#define set_bit(bitboard, square) (bitboard |= (1ULL << square))
#define delete_bit(bitboard, square) (get_bit(bitboard,square) ? bitboard ^= (1ULL << square):0)
using namespace std;

enum{
a8, b8, c8, d8, e8, f8, g8, h8,
a7, b7, c7, d7, e7, f7, g7, h7,
a6, b6, c6, d6, e6, f6, g6, h6,
a5, b5, c5, d5, e5, f5, g5, h5,
a4, b4, c4, d4, e4, f4, g4, h4,
a3, b3, c3, d3, e3, f3, g3, h3,
a2, b2, c2, d2, e2, f2, g2, h2,
a1, b1, c1, d1, e1, f1, g1, h1
};
/*
        NOT A FILE
 8  0  1  1  1  1  1  1  1
 7  0  1  1  1  1  1  1  1
 6  0  1  1  1  1  1  1  1
 5  0  1  1  1  1  1  1  1
 4  0  1  1  1  1  1  1  1
 3  0  1  1  1  1  1  1  1
 2  0  1  1  1  1  1  1  1
 1  0  1  1  1  1  1  1  1
    a  b  c  d  e  f  g  h
*/
/*
        NOT H FILE
 8  1  1  1  1  1  1  1  0
 7  1  1  1  1  1  1  1  0
 6  1  1  1  1  1  1  1  0
 5  1  1  1  1  1  1  1  0
 4  1  1  1  1  1  1  1  0
 3  1  1  1  1  1  1  1  0
 2  1  1  1  1  1  1  1  0
 1  1  1  1  1  1  1  1  0
    a  b  c  d  e  f  g  h
*/
/*
        NOT GH FILE
 8  1  1  1  1  1  1  0  0
 7  1  1  1  1  1  1  0  0
 6  1  1  1  1  1  1  0  0
 5  1  1  1  1  1  1  0  0
 4  1  1  1  1  1  1  0  0
 3  1  1  1  1  1  1  0  0
 2  1  1  1  1  1  1  0  0
 1  1  1  1  1  1  1  0  0
    a  b  c  d  e  f  g  h
*/
/*
        NOT AB FILE
 8  0  0  1  1  1  1  1  1
 7  0  0  1  1  1  1  1  1
 6  0  0  1  1  1  1  1  1
 5  0  0  1  1  1  1  1  1
 4  0  0  1  1  1  1  1  1
 3  0  0  1  1  1  1  1  1
 2  0  0  1  1  1  1  1  1
 1  0  0  1  1  1  1  1  1
    a  b  c  d  e  f  g  h
*/
/*
"a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
"a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
"a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
"a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
"a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
"a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
"a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
"a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1"
*/
const U64 not_a_file = 18374403900871474942ULL;
const U64 not_h_file = 9187201950435737471ULL;
const U64 not_gh_file = 4557430888798830399ULL;
const U64 not_ab_file = 18229723555195321596ULL;
U64 pawn_attacks[2][64];
U64 knight_attacks[64];
U64 king_attacks[64];
U64 bishop_attacks[64];
U64 rook_attacks[64];
void print_bitboard(U64 bitboard);
U64 mask_pawn_attacks(int side, int square);
U64 mask_knight_attacks(int square);
U64 mask_king_attacks(int square);
U64 mask_bishop_attacks(int square);
U64 mask_rook_attacks(int square);
void init_all_pawn_attacks();
void init_all_knight_attacks();
void init_all_king_attacks();
void init_all_bishop_attacks();
void init_all_rook_attacks();


int main()
{
    init_all_pawn_attacks();
    init_all_knight_attacks();
    init_all_king_attacks();
    init_all_bishop_attacks();
    init_all_rook_attacks();
    for(int i=0; i<BOARD_SIZE*BOARD_SIZE; i++)
    {
        print_bitboard(rook_attacks[i]);
    }
    std::cin.get();
    return 0;
}

void print_bitboard(U64 bitboard){
    for(int i=0; i<BOARD_SIZE; i++)
    {
        for(int j=0; j<BOARD_SIZE; j++)
        {
            if(!j)
            {
                cout<<' '<<8-i<<' ';
            }
            int square = (i*8)+j;
            cout<<' '<<get_bit(bitboard, square)<<' ';
        }
        cout<<endl;
        if(i==7)
        {
            for(char k='a'; k<='h'; k++)
            {
                if(k=='a')
                {
                    cout<<"    "<<k;
                }
                else
                {
                    cout<<"  "<<k;
                }
            }
            cout<<endl;
        }
    }
    cout<<"Bitboard: "<<bitboard<<endl;
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
void init_all_bishop_attacks(){
    for(int i=0; i<BOARD_SIZE*BOARD_SIZE; i++)
    {
        bishop_attacks[i] = mask_bishop_attacks(i);
        bishop_attacks[i] = mask_bishop_attacks(i);
    }
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
void init_all_rook_attacks(){
    for(int i=0; i<BOARD_SIZE*BOARD_SIZE; i++)
    {
        rook_attacks[i] = mask_rook_attacks(i);
        rook_attacks[i] = mask_rook_attacks(i);
    }
}













