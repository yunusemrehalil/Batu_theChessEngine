#pragma once
#ifndef PRINTFUNCS_CPP
#define PRINTFUNCS_CPP

#include "../headers/constants.hpp"
#include "../headers/arrays.hpp"
#include "namespace_testFuncs.cpp"

using namespace std;

namespace printFuncs{
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
        //cout<<"Current Evaluation: "<<evaluate()<< " | Current Tension: "<<tension()<<endl;
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
    void print_move(int move)
    {
        cout<<' '<<square_to_coordinate[get_move_source(move)]<<square_to_coordinate[get_move_target(move)]<<promoted_piece[get_move_promoted(move)];
    }
    void print_move_list(moves *move_list)
    {
        if(!move_list->count)
        {
            cout<<endl<<" No move in the list!"<<endl;
            return;
        }
        cout<<endl<<" move    piece    capture    doublepawn    enpassant    castling    checking    legality     score"<<endl;
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
                            <<"           "<<move_list->move_legality[move_count]
                            <<"            "<<move_list->move_score[move_count]<<endl;
        }
        cout<<" Total number of moves : "<<move_list->count<<endl;
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

}

#endif