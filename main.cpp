#include <iostream>
#include <chrono>
#include <algorithm>
#include <bits/stdc++.h>

#include "headers/constants.hpp"
#include "headers/magic_numbers.hpp"
#include "headers/enums.hpp"
#include "headers/arrays.hpp"
#include "namespaces/namespace_staticFuncs.cpp"
#include "namespaces/namespace_maskPieceFuncs.cpp"
#include "namespaces/namespace_initializeFuncs.cpp"
#include "namespaces/namespace_magicNumberFuncs.cpp"
#include "namespaces/namespace_testFuncs.cpp"

using namespace std;
using namespace sif;
using namespace maskPieceFuncs;
using namespace initializeFuncs;
using namespace magicNumberFuncs;
using namespace testFuncs;

void parse_fen(const char *fen);
void print_move_list(moves *move_list);
void print_move(int move);
void print_bitboard(U64 bitboard);
void print_chess_board();
void order_moves(moves *move_list);
void print_evaluated_move_list(vector<moves> move_list);

int main()
{
    cin.get();
    init_all();
    parse_fen(tricky_position);
    print_chess_board();
    moves move_list[1];
    generate_moves(move_list);
    vector<moves> evaluated_move_list(move_list->count);
    moves new_list = search_test(5, NEGATIVEINFINITY, POSITIVEINFINITY);
    for(int i = 0; i<move_list->count; i++)
    {
        evaluated_move_list[i].move_score[0] = new_list.move_score[i];
        evaluated_move_list[i].moves[0] = new_list.moves[i];
        if(side == WHITE)
        {
            if(evaluated_move_list[i].move_score[0] > best_move_WHITE_score)
            {
                best_move_WHITE = evaluated_move_list[i].moves[0];
                best_move_WHITE_score = evaluated_move_list[i].move_score[0];
            }
        }
        else
        {
            if(evaluated_move_list[i].move_score[0] < best_move_BLACK_score)
            {
                best_move_BLACK = evaluated_move_list[i].moves[0];
                best_move_BLACK_score = evaluated_move_list[i].move_score[0];
            }
        }  
    }
    (side == WHITE)? best_move = best_move_WHITE: best_move = best_move_BLACK;
    cout<<endl<<" Best move for "<<((side==WHITE)?"white: ":"black: ")<<square_to_coordinate[get_move_source(best_move)]
            <<square_to_coordinate[get_move_target(best_move)]<<promoted_piece[get_move_promoted(best_move)]<<endl;
    cin.get();
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
                fen++;
            }
            if(*fen == '/')
            {
                fen++;
            }
        }
    }
    fen++;
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
        fen++;
    }
    fen++;
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
    occupancy_bitboards[BOTH] |= occupancy_bitboards[WHITE];
    occupancy_bitboards[BOTH] |= occupancy_bitboards[BLACK];
}
void print_move(int move)
{
    cout<<' '<<square_to_coordinate[get_move_source(move)]<<square_to_coordinate[get_move_target(move)]<<promoted_piece[get_move_promoted(move)]<<endl;
}
void print_move_list(moves *move_list)
{
    if(!move_list->count)
    {
        cout<<endl<<" No move in the list!"<<endl;
        return;
    }
    cout<<endl<<" move    piece    capture    doublepawn    enpassant    castling    checking    score"<<endl;
    for(int move_count=0; move_count<move_list->count; move_count++)
    {
        int move = move_list->moves[move_count];
        cout<<' '<<square_to_coordinate[get_move_source(move)]<<square_to_coordinate[get_move_target(move)]<<(get_move_promoted(move)?promoted_piece[get_move_promoted(move)]:' ')<<"   "<<
                        ascii_pieces[get_move_piece(move)]
                        <<"        "<<(get_move_capture(move)?1:0)
                        <<"          "<<(get_move_doublepawn(move)?1:0)
                        <<"             "<<(get_move_enpassant(move)?1:0)
                        <<"            "<<(get_move_castling(move)?1:0)
                        <<"           "<<(get_move_checking(move)?1:0)
                        <<"           "<<move_list->move_score[move_count]<<endl;
    }
    cout<<" Total number of moves : "<<move_list->count<<endl;
}
void order_moves(moves *move_list){
    for(int i=0; i<move_list->count; i++)
    {
        int move = move_list->moves[i];
        int move_score_guess = 0;
        int source_piece = get_move_piece(move);
        int target_piece = get_captured_piece(move);
        if(get_move_capture(move))
        {
            move_score_guess = 10 * piece_value[target_piece] - piece_value[source_piece];
            cout<<ascii_pieces[source_piece]<<ascii_pieces[target_piece]<<": "<<move_score_guess<<endl;
        }
    }
}
void print_evaluated_move_list(vector<moves> move_list){
    cout<<endl<<" move    piece    capture    doublepawn    enpassant    castling    checking    score"<<endl;
    for(int move_count=0; move_count<move_list.size(); move_count++)
    {
        //move_list[move_count].moves[move_count];
        int move = move_list[move_count].moves[move_count];
        cout<<' '<<square_to_coordinate[get_move_source(move)]<<square_to_coordinate[get_move_target(move)]<<(get_move_promoted(move)?promoted_piece[get_move_promoted(move)]:' ')<<"   "<<
                        ascii_pieces[get_move_piece(move)]
                        <<"        "<<(get_move_capture(move)?1:0)
                        <<"          "<<(get_move_doublepawn(move)?1:0)
                        <<"             "<<(get_move_enpassant(move)?1:0)
                        <<"            "<<(get_move_castling(move)?1:0)
                        <<"           "<<(get_move_checking(move)?1:0)
                        <<"           "<<move_list[move_count].move_score[move_count]<<endl;
    }
    cout<<" Total number of moves : "<<move_list.size()<<endl;
}






