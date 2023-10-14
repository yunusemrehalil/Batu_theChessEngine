#include <iostream>
#include "headers/constants.hpp"
#include "headers/magic_numbers.hpp"
#include "headers/enums.hpp"
#include "headers/arrays.hpp"
#include "namespaces/namespace_staticFuncs.cpp"
#include "namespaces/namespace_maskPieceFuncs.cpp"
#include "namespaces/namespace_initializeFuncs.cpp"
#include "namespaces/namespace_magicNumberFuncs.cpp"

using namespace std;
using namespace sif;
using namespace maskPieceFuncs;
using namespace initializeFuncs;
using namespace magicNumberFuncs;

int side;
int enpassant = no_sq;
int castle;

void print_bitboard(U64 bitboard);
void print_chess_board();
void parse_fen(const char *fen);


int main()
{
    init_all();
    //WHITE PAWNS
    /*set_bit(piece_bitboards[P], a2);
    set_bit(piece_bitboards[P], b2);
    set_bit(piece_bitboards[P], c2);
    set_bit(piece_bitboards[P], d2);
    set_bit(piece_bitboards[P], e2);
    set_bit(piece_bitboards[P], f2);
    set_bit(piece_bitboards[P], g2);
    set_bit(piece_bitboards[P], h2);
    //WHITE PIECES
    set_bit(piece_bitboards[R], a1);
    set_bit(piece_bitboards[R], h1);
    set_bit(piece_bitboards[N], b1);
    set_bit(piece_bitboards[N], g1);
    set_bit(piece_bitboards[B], c1);
    set_bit(piece_bitboards[B], f1);
    set_bit(piece_bitboards[Q], d1);
    set_bit(piece_bitboards[K], e1);
    //BLACK PAWNS
    set_bit(piece_bitboards[p], a7);
    set_bit(piece_bitboards[p], b7);
    set_bit(piece_bitboards[p], c7);
    set_bit(piece_bitboards[p], d7);
    set_bit(piece_bitboards[p], e7);
    set_bit(piece_bitboards[p], f7);
    set_bit(piece_bitboards[p], g7);
    set_bit(piece_bitboards[p], h7);
    //BLACK PIECES
    set_bit(piece_bitboards[r], a8);
    set_bit(piece_bitboards[r], h8);
    set_bit(piece_bitboards[n], b8);
    set_bit(piece_bitboards[n], g8);
    set_bit(piece_bitboards[b], c8);
    set_bit(piece_bitboards[b], f8);
    set_bit(piece_bitboards[q], d8);
    set_bit(piece_bitboards[k], e8);*/
    //print_bitboard(piece_bitboards[R]);
    side = WHITE;
    castle = wk | wq | bk | bq;
    //print_chess_board();
    /*cout<<ascii_pieces[P]<<endl;
    cout<<ascii_pieces[char_pieces['K']];*/
    //print_bitboard(occupancy);
    U64 occupancy = 0ULL;
    set_bit(occupancy, e5);
    set_bit(occupancy, f5);
    set_bit(occupancy, c4);
    set_bit(occupancy, c2);
    set_bit(occupancy, e3);
    set_bit(occupancy, g4);
    set_bit(occupancy, c6);
    set_bit(occupancy, g2);
    print_bitboard(get_queen_attacks(e4, occupancy));
    parse_fen(start_position);
    print_chess_board();
    //print_bitboard(occupancy_bitboards[BLACK]);
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
void print_chess_board(){
    for(int i=0; i<BOARD_SIZE; i++)
    {
        for(int j=0; j<BOARD_SIZE; j++)
        {
            int square = i*8 + j;
            if(!j)
            {
                cout<<' '<<8-i<<' ';
            }
            int piece = -1;
            for(int bb_count = P; bb_count<=k; bb_count++)
            {
                if(get_bit(piece_bitboards[bb_count], square)) piece = bb_count;
            }
            cout<<' '<<((piece == -1)?'.':ascii_pieces[piece])<<' ';
        }
        cout<<endl;
        if(i==7)
        {
            //cout<<endl;
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
            cout<<endl<<endl;
        }
    }
    cout<<" Side: "<<((!side)?"white":"black")<<' '<<endl;
    cout<<" Enpassant: "<<((enpassant !=no_sq)?square_to_coordinate[enpassant]: "no")<<endl;
    cout<<" Castling: "<<((castle & wk)?'K':'-')<<((castle & wq)?"Q":"-")<<((castle & bk)?"k":"-")<<((castle & bq)?"q":"-")<<endl;
}
void parse_fen(const char *fen){
    memset(piece_bitboards, 0ULL, sizeof(piece_bitboards));
    memset(occupancy_bitboards, 0ULL, sizeof(occupancy_bitboards));
    side = 0;
    enpassant = no_sq;
    castle = 0;
    for(int i=0; i<BOARD_SIZE; i++)
    {
        for(int j=0; j<BOARD_SIZE; j++)
        {
            int square = i*8+j;
            if((*fen >='a' && *fen <='z') || (*fen >='A' && *fen <='Z'))
            {
                int piece  = char_pieces[*fen];
                set_bit(piece_bitboards[piece], square);
                fen++;
            }
            if((*fen >='0' && *fen <='9'))
            {
                int offset = *fen - '0';
                int piece = -1;
                for(int bb_count = P; bb_count<=k; bb_count++)
                {
                    if(get_bit(piece_bitboards[bb_count], square)) piece = bb_count;
                }
                if(piece == -1)
                {
                    j--;
                }
                j += offset;
                *fen++;
            }
            if(*fen == '/')
            {
                *fen++;
            }
        }
    }
    *fen++;
    (*fen =='w') ? (side=WHITE): side=BLACK;
    fen += 2;
    while(*fen != ' ')
    {
        switch (*fen)
        {
            case 'K': castle |= wk; break;
            case 'Q': castle |= wq; break;
            case 'k': castle |= bk; break;
            case 'q': castle |= bq; break;
            case '-': break;
        }
        *fen++;
    }
    *fen++;
    if(*fen != '-')
    {
        int j = fen[0] - 'a';
        int i = 8 - (fen[1] -'0');
        enpassant = i*8 + j;
    }
    else
    {
        enpassant = no_sq;
    }
    for(int piece = P; piece<= K; piece++)
    {
        occupancy_bitboards[WHITE] |= piece_bitboards[piece];
    }
    for(int piece = p; piece <= k; piece++)
    {
        occupancy_bitboards[BLACK] |= piece_bitboards[piece];
    }
}











