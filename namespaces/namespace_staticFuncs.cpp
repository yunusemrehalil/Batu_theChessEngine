#pragma once
#ifndef STATICFUNCS_CPP
#define STATICFUNCS_CPP

#include "../headers/constants.hpp"
#include "../headers/arrays.hpp"
#include <vector>
#include <string>
#include <cstring>
#include <algorithm>

int side;
int enpassant = no_sq;
int castle;
using namespace std;

namespace sif{
    //static inline functions
    static inline void add_move(moves *move_list, int move)
    {
        move_list->moves[move_list->count] = move;
        move_list->count++;
    }
    static inline void add_move_to_evaluated(moves *move_list, int move, int score)
    {
        move_list->moves[move_list->count] = move;
        move_list->move_score[move_list->count] = score;
        move_list->count++;
    }
    static inline void print_vector(vector<string> &v){
        for(int i = 0; i< v.size(); i++) {
            cout<<" "<<v[i]<<endl;
        }
    }
    static inline void print_2d_vector(vector<vector<string>> &v){
        for (int i = 0; i < v.size(); i++)  
        { 
            for (int j = 0; j < v[i].size(); j++) 
            { 
                cout<<" "<< v[i][j] << " "; 
            }     
            cout << endl; 
        } 
    }
    static inline void print_array(string a[], int count){
        for(int i=0; i<count; i++)
        {
            cout<<" "<<a[i]<<endl;
        }
    }
    static inline bool sort_check(moves *move_list){
        /*for(int i=0; i<move_list->count; i++)
        {

        }*/
        return 0;
    }
    static inline bool sort_capture(const string& str) {
        return str.find("x") != string::npos;
    }
    static inline bool sort_promotion(const string& str) {
        string is_promotion_square = str.substr(str.length() - 1);
        if(is_promotion_square == "Q" || is_promotion_square == "N" || is_promotion_square == "B" || is_promotion_square == "R") return 1;
        else return 0;
    }
    static inline bool sort_center_squares(const string& str){
        string is_center_square = str.substr(str.length() - 2);
        if(is_center_square == "d4" || is_center_square == "d5" || is_center_square == "e4" || is_center_square == "e5") return 1;
        else return 0;
        //return is_center_square;
    }
    static inline int count_bits(U64 bitboard){
        int count = 0;
        while(bitboard)
        {
            count++;
            bitboard &= bitboard -1;
        }
        return count;
    }
    static inline int get_1st_bit_index(U64 bitboard){
        if(bitboard)
        {
            return count_bits((bitboard & -bitboard)-1);
        }
        else return -1;
    }
    static inline U64 get_bishop_attacks(int square, U64 occupancy){
        occupancy &= bishop_masks[square];
        occupancy *= bishop_magic_numbers[square];
        occupancy >>= 64-bishop_relevant_bits[square];
        return bishop_attacks[square][occupancy];
    }
    static inline U64 get_rook_attacks(int square, U64 occupancy){
        occupancy &= rook_masks[square];
        occupancy *= rook_magic_numbers[square];
        occupancy >>= 64-rook_relevant_bits[square];
        return rook_attacks[square][occupancy];
    }
    static inline U64 get_queen_attacks(int square, U64 occupancy){
        U64 queen_attacks = 0ULL;
        U64 bishop_occupancy = occupancy;
        U64 rook_occupancy = occupancy;
        bishop_occupancy &= bishop_masks[square];
        bishop_occupancy *= bishop_magic_numbers[square];
        bishop_occupancy >>= 64 - bishop_relevant_bits[square];
        queen_attacks = bishop_attacks[square][bishop_occupancy];
        rook_occupancy &= rook_masks[square];
        rook_occupancy *= rook_magic_numbers[square];
        rook_occupancy >>= 64 - rook_relevant_bits[square];
        queen_attacks |= rook_attacks[square][rook_occupancy];
        return queen_attacks;
    }  
    static inline int is_square_attacked(int square, int side){
        if ((side == WHITE && pawn_attacks[BLACK][square] & piece_bitboards[P]) || (side == BLACK && pawn_attacks[WHITE][square] & piece_bitboards[p])) return 1;
        if(knight_attacks[square] & ((side==WHITE)?piece_bitboards[N]:piece_bitboards[n])) return 1;
        if(king_attacks[square] & ((side==WHITE)?piece_bitboards[K]:piece_bitboards[k])) return 1;
        if(get_bishop_attacks(square, occupancy_bitboards[BOTH])& ((side==WHITE)?piece_bitboards[B]:piece_bitboards[b])) return 1;
        if(get_rook_attacks(square, occupancy_bitboards[BOTH])& ((side==WHITE)?piece_bitboards[R]:piece_bitboards[r])) return 1;
        if(get_queen_attacks(square, occupancy_bitboards[BOTH])& ((side==WHITE)?piece_bitboards[Q]:piece_bitboards[q])) return 1;
        return 0;
    }
    void print_attacked_squares(int side){
        for(int i=0; i<BOARD_SIZE; i++)
        {
            for(int j=0; j<BOARD_SIZE; j++)
            {
                if(!j)
                {
                    cout<<' '<<8-i<<' ';
                }
                int square = (i*8)+j;
                cout<<" "<<(is_square_attacked(square, side)?1:0)<<" ";
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
    }
    static inline void generate_moves(moves *move_list){
        move_list->count = 0;
        int source_square, target_square, king;
        int check_count = 0, center_captures_count = 0, promote_captures_count = 0, 
                captures_count = 0, centers_count = 0, promotions_count = 0, others_count = 0 ;
        king = (side == WHITE) ? k : K; 
        U64 bitboard, attacks;
        for(int piece=P; piece<=k; piece++)
        {
            bitboard = piece_bitboards[piece];
            //white pawns and white king castling moves
            if(side == WHITE)
            {
                if(piece == P)
                {
                    while(bitboard)
                    {
                        source_square = get_1st_bit_index(bitboard);
                        target_square = source_square - 8;
                        //check and quite pawn moves
                        if(!(target_square < a8) && !get_bit(occupancy_bitboards[BOTH], target_square))
                        {
                            //promotion
                            if(source_square >= a7 && source_square <= h7)
                            {
                                /*for(int promotion : promoted_piece) PROMOTİON SON HARFİ DÖNGÜYLE DE YAPILABİLİR
                                {
                                    /*string move = string(string(square_to_coordinate[source_square]) + string(square_to_coordinate[target_square]) + 
                                                                promotion);
                                    legal_moves.push_back(move);
                                    int new_move = encode_move(source_square, target_square, piece, promotion, 0, 0, 0, 0);

                                    add_move(move_list, new_move);
                                    /*add_move(move_list, new_move_2);
                                    add_move(move_list, new_move_3);
                                    add_move(move_list, new_move_4);
                                }*/
                                add_move(move_list, encode_move(source_square, target_square, piece, Q, 0, 0, 0, 0, 0));
                                add_move(move_list, encode_move(source_square, target_square, piece, R, 0, 0, 0, 0, 0));
                                add_move(move_list, encode_move(source_square, target_square, piece, B, 0, 0, 0, 0, 0));
                                add_move(move_list, encode_move(source_square, target_square, piece, N, 0, 0, 0, 0, 0));
                            }
                            else
                            {
                                //1 square ahead and check
                                if(piece_bitboards[k] & pawn_attacks[side][target_square])
                                {
                                    
                                    /*string move = string(string(square_to_coordinate[source_square]) + string(square_to_coordinate[target_square]) + "+");
                                    legal_moves.push_back(move);*/
                                    add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0, 1));
                                }
                                else
                                {
                                    //1 square ahead
                                    /*string move = string(string(square_to_coordinate[source_square]) + string(square_to_coordinate[target_square]));
                                    legal_moves.push_back(move);*/
                                    add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0, 0));
                                }
                                if((source_square >= a2 && source_square <= h2) && !get_bit(occupancy_bitboards[BOTH], target_square-8))
                                {
                                    //2 square ahead and check
                                    if(piece_bitboards[k] & pawn_attacks[side][target_square-8])
                                    {
                                        /*string move = string(string(square_to_coordinate[source_square]) + string(square_to_coordinate[target_square-8]) + "+");
                                        legal_moves.push_back(move);*/
                                        add_move(move_list, encode_move(source_square, target_square-8, piece, 0, 0, 1, 0, 0, 1));
                                    }
                                    else
                                    {
                                        //2 square ahead
                                        add_move(move_list, encode_move(source_square, target_square-8, piece, 0, 0, 1, 0, 0, 0));
                                        /*if((source_square >= a2 && source_square <= h2) && !get_bit(occupancy_bitboards[BOTH], target_square-8))
                                        {
                                            /*string move = string(string(square_to_coordinate[source_square]) + string(square_to_coordinate[target_square-8]));
                                            legal_moves.push_back(move);
                                            add_move(move_list, encode_move(source_square, target_square-8, piece, 0, 0, 1, 0, 0, 0));
                                        }*/
                                    }
                                }
                            }
                        }
                        //pawn captures
                        attacks = pawn_attacks[side][source_square] & occupancy_bitboards[BLACK];
                        while (attacks)
                        {
                            target_square = get_1st_bit_index(attacks);
                            //capture and promotion
                            if(source_square >= a7 && source_square <= h7)
                            {
                                /*for(char promotion : white_promotions)
                                {
                                    string move = string(string(square_to_coordinate[source_square]) + "x" + string(square_to_coordinate[target_square]) + 
                                                                promotion);
                                    legal_moves.push_back(move);
                                }*/
                                add_move(move_list, encode_move(source_square, target_square, piece, Q, 1, 0, 0, 0, 0));
                                add_move(move_list, encode_move(source_square, target_square, piece, R, 1, 0, 0, 0, 0));
                                add_move(move_list, encode_move(source_square, target_square, piece, B, 1, 0, 0, 0, 0));
                                add_move(move_list, encode_move(source_square, target_square, piece, N, 1, 0, 0, 0, 0));
                            }
                            else
                            {
                                //1 square ahead capture and check
                                if(piece_bitboards[k] & pawn_attacks[side][target_square])
                                {
                                    /*string move = string(string(square_to_coordinate[source_square]) + "x" +  string(square_to_coordinate[target_square]) + "+");
                                    legal_moves.push_back(move);*/
                                    add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0, 1));
                                }
                                //1 square ahead capture
                                else
                                {
                                    /*string move = string(string(square_to_coordinate[source_square]) + "x" +  string(square_to_coordinate[target_square]));
                                    legal_moves.push_back(move);*/
                                    add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0, 0));
                                }

                            }
                            delete_bit(attacks, target_square);
                        }
                        if(enpassant != no_sq)
                        {
                            U64 enpassant_attacks = pawn_attacks[side][source_square] & (1ULL << enpassant);
                            if(enpassant_attacks)
                            {
                                int target_enpassant = get_1st_bit_index(enpassant_attacks); 
                                //enpassant and check
                                if(piece_bitboards[k] & pawn_attacks[side][target_enpassant])
                                {
                                    /*string move = string(string(square_to_coordinate[source_square]) + "x" + string(square_to_coordinate[target_enpassant]) + "+");
                                    legal_moves.push_back(move);   */
                                    add_move(move_list, encode_move(source_square, target_enpassant, piece, 0, 1, 0, 1, 0, 1)); 
                                }
                                //quite enpassant
                                else
                                {
                                    /*string move = string(string(square_to_coordinate[source_square]) + "x" +string(square_to_coordinate[target_enpassant]));
                                    legal_moves.push_back(move);*/
                                    add_move(move_list, encode_move(source_square, target_enpassant, piece, 0, 1, 0, 1, 0, 0));   
                                }
                            }
                        }
                        delete_bit(bitboard, source_square);
                    }
                }
                //castling
                if(piece == K)
                {
                    //king side castling
                    if(castle & wk)
                    {
                        if(!(get_bit(occupancy_bitboards[BOTH], f1)) && !(get_bit(occupancy_bitboards[BOTH], g1)))
                        {
                            if(!is_square_attacked(e1, BLACK) && !is_square_attacked(f1, BLACK))//&& !is_square_attacked(g1, BLACK)
                            {
                                /*string move = "O-O : e1g1";
                                legal_moves.push_back(move);*/
                                add_move(move_list, encode_move(e1, g1, piece, 0, 0, 0, 0, 1, 0));
                            }
                        }
                    }
                    //queen side castling
                    if(castle & wq)
                    {
                        if(!(get_bit(occupancy_bitboards[BOTH], d1)) && !(get_bit(occupancy_bitboards[BOTH], c1)) && !(get_bit(occupancy_bitboards[BOTH], b1)))
                        {
                            if(!is_square_attacked(e1, BLACK) && !is_square_attacked(d1, BLACK))//&& !is_square_attacked(c1, BLACK)
                            {
                                /*string move = "O-O-O : e1c1";
                                legal_moves.push_back(move);*/
                                add_move(move_list, encode_move(e1, c1, piece, 0, 0, 0, 0, 1, 0));
                            }
                        }
                    }
                }
            }
            //black pawns and black king castling moves
            else
            {
                if(piece == p)
                {
                    while(bitboard)
                    {
                        source_square = get_1st_bit_index(bitboard);
                        target_square = source_square + 8;
                        //check and quite pawn moves
                        if(!(target_square > h1) && !get_bit(occupancy_bitboards[BOTH], target_square))
                        {
                            //promotion
                            if(source_square >= a2 && source_square <= h2)
                            {
                                /*for(char promotion : black_promotions)
                                {
                                    string move = string(string(square_to_coordinate[source_square]) + string(square_to_coordinate[target_square]) + 
                                                                promotion);
                                    legal_moves.push_back(move);
                                }*/
                                add_move(move_list, encode_move(source_square, target_square, piece, q, 0, 0, 0, 0, 0));
                                add_move(move_list, encode_move(source_square, target_square, piece, r, 0, 0, 0, 0, 0));
                                add_move(move_list, encode_move(source_square, target_square, piece, b, 0, 0, 0, 0, 0));
                                add_move(move_list, encode_move(source_square, target_square, piece, n, 0, 0, 0, 0, 0));
                            }
                            else
                            {
                                //1 square ahead and check
                                if(piece_bitboards[K] & pawn_attacks[side][target_square])
                                {
                                    /*string move = string(string(square_to_coordinate[source_square]) + string(square_to_coordinate[target_square]) + "+");
                                    legal_moves.push_back(move);*/
                                    add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0, 1));
                                }
                                else
                                {
                                    //1 square ahead
                                    /*string move = string(string(square_to_coordinate[source_square]) + string(square_to_coordinate[target_square]));
                                    legal_moves.push_back(move);*/
                                    add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0, 0));
                                }
                                if((source_square >= a7 && source_square <= h7) && !get_bit(occupancy_bitboards[BOTH], target_square+8))
                                {
                                    //2 square ahead and check
                                    if(piece_bitboards[K] & pawn_attacks[side][target_square+8])
                                    {
                                        /*string move = string(string(square_to_coordinate[source_square]) + string(square_to_coordinate[target_square-8]) + "+");
                                        legal_moves.push_back(move);*/
                                        add_move(move_list, encode_move(source_square, target_square+8, piece, 0, 0, 1, 0, 0, 1));
                                    }
                                    else
                                    {
                                        //2 square ahead
                                        add_move(move_list, encode_move(source_square, target_square+8, piece, 0, 0, 1, 0, 0, 0));
                                        /*if((source_square >= a7 && source_square <= h7) && !get_bit(occupancy_bitboards[BOTH], target_square+8))
                                        {
                                            string move = string(string(square_to_coordinate[source_square]) + string(square_to_coordinate[target_square-8]));
                                            legal_moves.push_back(move);
                                            add_move(move_list, encode_move(source_square, target_square+8, piece, 0, 0, 1, 0, 0, 0));
                                        }*/
                                    }
                                }
                            }
                        }
                        //pawn captures
                        attacks = pawn_attacks[side][source_square] & occupancy_bitboards[WHITE];
                        while (attacks)
                        {
                            target_square = get_1st_bit_index(attacks);
                            if(source_square >= a2 && source_square <= h2)
                            {
                                /*for(char promotion : black_promotions)
                                {
                                    string move = string(string(square_to_coordinate[source_square]) + "x" + string(square_to_coordinate[target_square]) + 
                                                                promotion);
                                    legal_moves.push_back(move);
                                }*/
                                add_move(move_list, encode_move(source_square, target_square, piece, q, 1, 0, 0, 0, 0));
                                add_move(move_list, encode_move(source_square, target_square, piece, r, 1, 0, 0, 0, 0));
                                add_move(move_list, encode_move(source_square, target_square, piece, b, 1, 0, 0, 0, 0));
                                add_move(move_list, encode_move(source_square, target_square, piece, n, 1, 0, 0, 0, 0));
                            }
                            else
                            {
                                //1 square ahead capture and check
                                if(piece_bitboards[K] & pawn_attacks[side][target_square])
                                {
                                    /*string move = string(string(square_to_coordinate[source_square]) + "x" +  string(square_to_coordinate[target_square]) + "+");
                                    legal_moves.push_back(move);*/
                                    add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0, 1));
                                }
                                //1 square capture ahead
                                else
                                {
                                    /*string move = string(string(square_to_coordinate[source_square]) + "x" +  string(square_to_coordinate[target_square]));
                                    legal_moves.push_back(move);*/
                                    add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0, 0));
                                }
                            }
                            delete_bit(attacks, target_square);
                        }
                        if(enpassant != no_sq)
                        {
                            U64 enpassant_attacks = pawn_attacks[side][source_square] & (1ULL << enpassant);
                            if(enpassant_attacks)
                            {
                                int target_enpassant = get_1st_bit_index(enpassant_attacks);               
                                //enpassant and check
                                if(piece_bitboards[K] & pawn_attacks[side][target_enpassant])
                                {
                                    /*string move = string(string(square_to_coordinate[source_square]) + "x" +string(square_to_coordinate[target_enpassant]) + "+");
                                    legal_moves.push_back(move); */   
                                    add_move(move_list, encode_move(source_square, target_enpassant, piece, 0, 1, 0, 1, 0, 1)); 
                                }
                                //quiet enpassant
                                else
                                {
                                    /*string move = string(string(square_to_coordinate[source_square]) + "x" +string(square_to_coordinate[target_enpassant]));
                                    legal_moves.push_back(move);*/
                                    add_move(move_list, encode_move(source_square, target_enpassant, piece, 0, 1, 0, 1, 0, 0));
                                }
                            }
                        }
                        delete_bit(bitboard, source_square);
                    }
                }
                //castling
                if(piece == k)
                {
                    //king side castling
                    if(castle & bk)
                    {
                        if(!(get_bit(occupancy_bitboards[BOTH], f8)) && !(get_bit(occupancy_bitboards[BOTH], g8)))
                        {
                            if(!is_square_attacked(e8, WHITE) && !is_square_attacked(f8, WHITE)) //&& !is_square_attacked(g8, WHITE)
                            {
                                /*string move = "O-O : e8g8";
                                legal_moves.push_back(move);*/
                                add_move(move_list, encode_move(e8, g8, piece, 0, 0, 0, 0, 1, 0));
                            }
                        }
                    }
                    //queen side castling
                    if(castle & bq)
                    {
                        if(!(get_bit(occupancy_bitboards[BOTH], b8)) && !(get_bit(occupancy_bitboards[BOTH], c8)) && !(get_bit(occupancy_bitboards[BOTH], d8)))
                        {
                            if(!is_square_attacked(e8, WHITE) && !is_square_attacked(d8, WHITE))//&& !is_square_attacked(c8, WHITE)
                            {
                                /*string move = "O-O-O : e8c8";
                                legal_moves.push_back(move);*/
                                add_move(move_list, encode_move(e8, c8, piece, 0, 0, 0, 0, 1, 0));
                            }
                        }
                    }
                }
            }
            
            
            //knigt moves
            if((side == WHITE)?(piece == N ):(piece == n))
            {
                while(bitboard)
                {
                    source_square = get_1st_bit_index(bitboard);
                    attacks = knight_attacks[source_square] & ((side == WHITE)? ~occupancy_bitboards[WHITE]: ~occupancy_bitboards[BLACK]);
                    while (attacks)
                    {
                        target_square = get_1st_bit_index(attacks);                        
                        //quiet move
                        /*if(!get_bit(((side == WHITE)?occupancy_bitboards[BLACK]:occupancy_bitboards[WHITE]),target_square))
                        {
                            string move = string(string(square_to_coordinate[source_square]) + string(square_to_coordinate[target_square]));
                            legal_moves.push_back(move);
                            //quite check
                            if(piece_bitboards[king] & knight_attacks[target_square])
                            {
                                string move = string(string(square_to_coordinate[source_square]) + string(square_to_coordinate[target_square]) + "+");
                                legal_moves.push_back(move);
                            }
                        }
                        //capture move
                        else
                        {
                            //capture check
                            if(piece_bitboards[king] & knight_attacks[target_square])
                            {
                                string move = string(string(square_to_coordinate[source_square]) + "x" + string(square_to_coordinate[target_square]) + "+");
                                legal_moves.push_back(move);
                            }
                            //capture
                            else
                            {
                                string move = string(string(square_to_coordinate[source_square]) + "x" +string(square_to_coordinate[target_square]));
                                legal_moves.push_back(move);
                            }
                            
                        }*/
                        /*string source = square_to_coordinate[source_square];
                        string target = square_to_coordinate[target_square];*/
                        //quite move
                        if (!get_bit(occupancy_bitboards[side == WHITE ? BLACK : WHITE], target_square)) 
                        {
                            /*legal_moves.push_back(source + target);
                            //quiet check move
                            if (piece_bitboards[king] & knight_attacks[target_square]) 
                            {
                                legal_moves.back() += "+";
                            }*/
                            add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0, 0));
                        }
                        //capture move
                        else 
                        {
                            //string capture_move = source + "x" + target;
                            //capture check move
                            if (piece_bitboards[king] & knight_attacks[target_square]) 
                            {
                                add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0, 1));
                            }
                            //legal_moves.push_back(capture_move);
                            else
                            {
                                add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0, 0));
                            }
                        }
                        delete_bit(attacks, target_square);    
                    }
                    delete_bit(bitboard, source_square);    
                }
            }
            //bishop moves
            if((side == WHITE)?(piece == B):(piece == b))
            {
                while(bitboard)
                {
                    source_square = get_1st_bit_index(bitboard);
                    attacks = get_bishop_attacks(source_square, occupancy_bitboards[BOTH]) & ((side == WHITE)? ~occupancy_bitboards[WHITE]: ~occupancy_bitboards[BLACK]);
                    while (attacks)
                    {
                        target_square = get_1st_bit_index(attacks);
                        /*string source = square_to_coordinate[source_square];
                        string target = square_to_coordinate[target_square];*/
                        //quite move
                        if (!get_bit(occupancy_bitboards[side == WHITE ? BLACK : WHITE], target_square)) 
                        {
                            /*legal_moves.push_back(source + target);
                            //quiet check move
                            if (piece_bitboards[king] & get_bishop_attacks(target_square, occupancy_bitboards[BOTH])) 
                            {
                                legal_moves.back() += "+";
                            }*/
                            add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0, 0));
                        }
                        //capture move
                        else 
                        {
                            //string capture_move = source + "x" + target;
                            //capture check move
                            if (piece_bitboards[king] & get_bishop_attacks(target_square, occupancy_bitboards[BOTH])) 
                            {
                                add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0, 1));
                            }
                            /*legal_moves.push_back(capture_move);*/
                            else
                            {
                                add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0, 0));
                            }
                            
                        }
                        /*//capture and check
                        if(piece_bitboards[king] & get_bishop_attacks(target_square, occupancy_bitboards[BOTH]))
                        {
                            string move = string(string(square_to_coordinate[source_square]) + "x" + string(square_to_coordinate[target_square]) + "+");
                            legal_moves.push_back(move);
                        }
                        //quiet move
                        else if(!get_bit(((side == WHITE)?occupancy_bitboards[BLACK]:occupancy_bitboards[WHITE]),target_square))
                        {
                            string move = string(string(square_to_coordinate[source_square]) + string(square_to_coordinate[target_square]));
                            legal_moves.push_back(move);
                        }
                        //capture move
                        else
                        {
                            string move = string(string(square_to_coordinate[source_square]) + "x" +  string(square_to_coordinate[target_square]));
                            legal_moves.push_back(move);
                        }*/
                        delete_bit(attacks, target_square);    
                    }
                    delete_bit(bitboard, source_square);    
                }
            }
            //rook moves
            if((side == WHITE)?(piece == R):(piece == r))
            {
                while(bitboard)
                {
                    source_square = get_1st_bit_index(bitboard);
                    attacks = get_rook_attacks(source_square, occupancy_bitboards[BOTH]) & ((side == WHITE)? ~occupancy_bitboards[WHITE]: ~occupancy_bitboards[BLACK]);
                    while (attacks)
                    {
                        target_square = get_1st_bit_index(attacks);
                        /*string source = square_to_coordinate[source_square];
                        string target = square_to_coordinate[target_square];*/
                        //quite move
                        if (!get_bit(occupancy_bitboards[side == WHITE ? BLACK : WHITE], target_square)) 
                        {
                            /*legal_moves.push_back(source + target);
                            //quiet check move
                            if (piece_bitboards[king] & get_rook_attacks(target_square, occupancy_bitboards[BOTH])) 
                            {
                                legal_moves.back() += "+";
                            }*/
                            add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0, 0));
                        }
                        //capture move
                        else 
                        {
                            //string capture_move = source + "x" + target;
                            //capture check move
                            if (piece_bitboards[king] & get_rook_attacks(target_square, occupancy_bitboards[BOTH])) 
                            {
                                add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0, 1));
                            }
                            /*legal_moves.push_back(capture_move);*/
                            else
                            {
                                add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0, 0));
                            }
                        }
                        /*//capture and check
                        if(piece_bitboards[king] & get_rook_attacks(target_square, occupancy_bitboards[BOTH]))
                        {
                            string move = string(string(square_to_coordinate[source_square]) + "x" +  string(square_to_coordinate[target_square]) + "+");
                            legal_moves.push_back(move);
                        }
                        //quiet move
                        else if(!get_bit(((side == WHITE)?occupancy_bitboards[BLACK]:occupancy_bitboards[WHITE]),target_square))
                        {
                            string move = string(string(square_to_coordinate[source_square]) + string(square_to_coordinate[target_square]));
                            legal_moves.push_back(move);
                        }
                        //capture move
                        else
                        {
                            string move = string(string(square_to_coordinate[source_square]) + "x" +  string(square_to_coordinate[target_square]));
                            legal_moves.push_back(move);
                        }*/
                        delete_bit(attacks, target_square);    
                    }
                    delete_bit(bitboard, source_square);    
                }
            }
            //queen moves
            if((side == WHITE)?(piece == Q): (piece == q))
            {
                while(bitboard)
                {
                    source_square = get_1st_bit_index(bitboard);
                    attacks = get_queen_attacks(source_square, occupancy_bitboards[BOTH]) & ((side == WHITE)? ~occupancy_bitboards[WHITE]: ~occupancy_bitboards[BLACK]);
                    while (attacks)
                    {
                        target_square = get_1st_bit_index(attacks);
                        /*string source = square_to_coordinate[source_square];
                        string target = square_to_coordinate[target_square];*/
                        //quite move
                        if (!get_bit(occupancy_bitboards[side == WHITE ? BLACK : WHITE], target_square)) 
                        {
                            /*legal_moves.push_back(source + target);
                            //quiet check move
                            if (piece_bitboards[king] & get_queen_attacks(target_square, occupancy_bitboards[BOTH])) 
                            {
                                legal_moves.back() += "+";
                            }*/
                            add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0, 0));
                        }
                        //capture move
                        else 
                        {
                            //string capture_move = source + "x" + target;
                            //capture check move
                            if (piece_bitboards[king] & get_queen_attacks(target_square, occupancy_bitboards[BOTH])) 
                            {
                                add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0, 1));
                            }
                            /*legal_moves.push_back(capture_move);*/
                            else
                            {
                                add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0, 0));
                            }
                        }
                        /*//capture and check
                        if(piece_bitboards[king] & get_queen_attacks(target_square, occupancy_bitboards[BOTH]))
                        {
                            string move = string(string(square_to_coordinate[source_square]) + "x" +  string(square_to_coordinate[target_square]) + "+");
                            legal_moves.push_back(move);
                        }
                        //quiet move
                        else if(!get_bit(((side == WHITE)?occupancy_bitboards[BLACK]:occupancy_bitboards[WHITE]),target_square))
                        {
                            string move = string(string(square_to_coordinate[source_square]) + string(square_to_coordinate[target_square]));
                            legal_moves.push_back(move);
                        }
                        //capture move
                        else
                        {
                            string move = string(string(square_to_coordinate[source_square]) + "x" +  string(square_to_coordinate[target_square]));
                            legal_moves.push_back(move);
                        }*/
                        delete_bit(attacks, target_square);    
                    }
                    delete_bit(bitboard, source_square);    
                }
            }
            //king moves
            if((side == WHITE)?piece == K: piece == k)
            {
                while(bitboard)
                {
                    source_square = get_1st_bit_index(bitboard);
                    attacks = king_attacks[source_square] & ((side == WHITE)? ~occupancy_bitboards[WHITE]: ~occupancy_bitboards[BLACK]);
                    while (attacks)
                    {
                        target_square = get_1st_bit_index(attacks);
                        //quiet move
                        if(!get_bit(((side == WHITE)?occupancy_bitboards[BLACK]:occupancy_bitboards[WHITE]),target_square))
                        {
                            /*string move = string(string(square_to_coordinate[source_square]) + string(square_to_coordinate[target_square]));
                            legal_moves.push_back(move);*/
                            add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0, 0));
                        }
                        //capture move
                        else
                        {
                            /*string move = string(string(square_to_coordinate[source_square]) + "x" +  string(square_to_coordinate[target_square]));
                            legal_moves.push_back(move);*/
                            add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0, 0));
                        }
                        delete_bit(attacks, target_square);    
                    }
                    delete_bit(bitboard, source_square);    
                }
            }
        }
        /*sort_vector(legal_moves);
        print_vector(legal_moves);*/
        //print_2d_vector(sortedMoves);
    }
    static inline int make_move(int move, int move_flag){
        //quite moves
        if(move_flag == all_moves)
        {
            //preserve board state
            copy_board();
            //parse move
            int source_square = get_move_source(move);
            int target_square = get_move_target(move);
            int piece = get_move_piece(move);
            int promoted_piece = get_move_promoted(move);
            int capture = get_move_capture(move);
            int doublepawn = get_move_doublepawn(move);
            int enpassant_parse = get_move_enpassant(move);
            int castle_parse = get_move_castling(move);
            //move piece
            delete_bit(piece_bitboards[piece], source_square);
            set_bit(piece_bitboards[piece], target_square);
            //handling capture moves
            if(capture)
            {
                //pick up bitboard piece index ranges depending on side
                int start_piece, end_piece;
                if(side == WHITE)
                {
                    start_piece = p;
                    end_piece = k;
                }
                else
                {
                    start_piece = P;
                    end_piece = K;
                }
                for(int bb_piece = start_piece; bb_piece<=end_piece; bb_piece++)
                {
                    if(get_bit(piece_bitboards[bb_piece], target_square))
                    {
                        delete_bit(piece_bitboards[bb_piece], target_square);
                        break;
                    }
                }
            }
            //handling promotions
            if(promoted_piece)
            {
                //erase the pawn from the target square
                delete_bit(piece_bitboards[(side==WHITE)?P:p], target_square);
                //set up promoted piece to board
                set_bit(piece_bitboards[promoted_piece], target_square);
            }
            //handle enpassant captures
            if(enpassant_parse) //////////////KONTROL ETTTTTTTTTTTT
            {
                //int kings_square = (side==WHITE)?get_1st_bit_index(piece_bitboards[K]):get_1st_bit_index(piece_bitboards[k]);
                (side==WHITE)?delete_bit(piece_bitboards[p], target_square+8):delete_bit(piece_bitboards[P], target_square-8);
                //cout<<endl<<square_to_coordinate[kings_square]<<"  "<<is_square_attacked(kings_square, side)<<endl;
                /*if(!is_square_attacked(kings_square, side))
                {
                    (side==WHITE)?delete_bit(piece_bitboards[p], target_square+8):delete_bit(piece_bitboards[P], target_square-8);
                    cout<<endl<<square_to_coordinate[kings_square]<<"  "<<is_square_attacked(kings_square, side)<<endl;
                }*/
            }
            enpassant = no_sq;
            //handle double pawn and enpassant
            if(doublepawn)
            {
                (side==WHITE)?(enpassant = target_square +8):(enpassant = target_square -8);
            }
            //handle castling
            if(castle_parse)
            {
                switch(target_square)
                {
                //wk
                case(g1):
                    delete_bit(piece_bitboards[R], h1);
                    set_bit(piece_bitboards[R], f1);
                    break;
                //wq
                case(c1):
                    delete_bit(piece_bitboards[R], a1);
                    set_bit(piece_bitboards[R], d1);
                    break;
                //bk
                case(g8):
                    delete_bit(piece_bitboards[r], h8);
                    set_bit(piece_bitboards[r], f8);
                    break;
                //bq
                case(c8):
                    delete_bit(piece_bitboards[r], a8);
                    set_bit(piece_bitboards[r], d8);
                    break;
                }
            }
            //update castling rights
            castle &= castling_rights[source_square];
            castle &= castling_rights[target_square];
            //reset occupancies
            memset(occupancy_bitboards, 0ULL, sizeof(occupancy_bitboards));
            //loop over white pieces bitboard
            for(int bb_piece =P; bb_piece <=K; bb_piece++)
            {
                //update white occupancies
                occupancy_bitboards[WHITE] |= piece_bitboards[bb_piece];
            }
            //loop over black pieces bitboard
            for(int bb_piece =p; bb_piece <=k; bb_piece++)
            {
                //update black occupancies
                occupancy_bitboards[BLACK] |= piece_bitboards[bb_piece];
            }
            //update both side occupancies
            occupancy_bitboards[BOTH] |= occupancy_bitboards[WHITE];
            occupancy_bitboards[BOTH] |= occupancy_bitboards[BLACK];
            //////check control
            //change side
            side ^= 1;
            if(is_square_attacked((side==WHITE)?get_1st_bit_index(piece_bitboards[k]):get_1st_bit_index(piece_bitboards[K]),side))
            {
                //take move back
                take_back();
                //return illegal move
                return 0;
            }
            else
            {
                //return legal move
                return 1;
            }

        }
        ////////////
        /////////////
        //////////////
        ///////////////
        ////////////////
        ////////////////BURALARDA YANLIS OLABILIRRRRRR
        ///////////////
        //////////////
        ////////////
        //////////
        //capture moves
        else
        {
            //make sure move is capture
            if(get_move_capture(move))
            {
                return make_move(move, all_moves);
            }
            //dont make the move
            else return 0;
        }
        return 0;
    }
    static inline void sort_move_list(moves *move_list){
        int n = move_list->count;
        //cout<<n<<endl;
        sort(move_list->move_score, (move_list->move_score)+n, greater<int>());
    
    //cout << "Array after sorting : \n";
    //for (int i = 0; i < n; ++i)
        //cout <<i+1<<". hamle: "<<square_to_coordinate[get_move_source(move_list->moves[i])]<<square_to_coordinate[get_move_target(move_list->moves[i])]<<promoted_piece[get_move_promoted(move_list->moves[i])]<< " "<<endl;
    /*int center_move[64], checking[64], capture[64], promotion[64], j=0,k=0,l=0,m=0;
    for(int i=0; i<move_list->count; i++)
    {
        int move = move_list->moves[i];
        if (get_move_target(move) == e4 ||
            get_move_target(move) == e5 ||
            get_move_target(move) == d4 ||
            get_move_target(move) == d5)
            {
                center_move[j] = move;
                //center_move[j] = string(square_to_coordinate[get_move_source(move)]);
                //cout<<" Merkeze Yapilan Hamle:";
                //cout<<' '<<square_to_coordinate[get_move_source(move)]<<square_to_coordinate[get_move_target(move)]<<promoted_piece[get_move_promoted(move)]<<endl;
                //cout<<center_move[j]<<endl;
                j++;
            }
        if(get_move_checking(move))
        {
            checking[k] = move;
            //cout<<" Sah Hamlesi:";
            //cout<<' '<<square_to_coordinate[get_move_source(move)]<<square_to_coordinate[get_move_target(move)]<<promoted_piece[get_move_promoted(move)]<<endl;
            k++;
        }
        if(get_move_capture(move))
        {
            capture[l] = move;
            //cout<<" Alis Hamlesi:";
            //cout<<' '<<square_to_coordinate[get_move_source(move)]<<square_to_coordinate[get_move_target(move)]<<promoted_piece[get_move_promoted(move)]<<endl;
            l++;
        }
        if(get_move_promoted(move))
        {
            promotion[m] = move;
            //cout<<" Promotion Hamlesi";
            //cout<<' '<<square_to_coordinate[get_move_source(move)]<<square_to_coordinate[get_move_target(move)]<<promoted_piece[get_move_promoted(move)]<<endl;
            m++;
        }
        for(int b=0; b<64; b++)
        {
            cout<<center_move[b]<<endl;
        }
    }*/
}
    static inline int get_captured_piece(int move){
        int target_piece = P;
        int target_square = get_move_target(move);
        int start_piece, end_piece;
        if(side == WHITE)
        {
            start_piece = p;
            end_piece = k;
        }
        else
        {
            start_piece = P;
            end_piece = K;
        }
        for(int bb_piece = start_piece; bb_piece<=end_piece; bb_piece++)
        {
            if(get_bit(piece_bitboards[bb_piece], target_square))
            {
                target_piece = bb_piece;
                break;
            }
        }
        return target_piece;
    }
}
#endif