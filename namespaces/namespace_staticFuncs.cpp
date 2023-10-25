#pragma once
#ifndef STATICFUNCS_CPP
#define STATICFUNCS_CPP
#include "../headers/constants.hpp"
#include "../headers/arrays.hpp"

int side;
int enpassant = no_sq;
int castle;

namespace sif{
    //static inline functions
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
    static inline void generate_moves(){
        int source_square, target_square;
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
                        //cout<<" white pawn : "<<square_to_coordinate[source_square]<<endl;
                        target_square = source_square - 8;
                        //quite pawn moves
                        if(!(target_square < a8) && !get_bit(occupancy_bitboards[BOTH], target_square))
                        {
                            //promotion
                            if(source_square >= a7 && source_square <= h7)
                            {
                                cout<<" white pawn promotion : "<<square_to_coordinate[source_square]<<square_to_coordinate[target_square]<<'Q'<<endl;
                                cout<<" white pawn promotion : "<<square_to_coordinate[source_square]<<square_to_coordinate[target_square]<<'R'<<endl;
                                cout<<" white pawn promotion : "<<square_to_coordinate[source_square]<<square_to_coordinate[target_square]<<'B'<<endl;
                                cout<<" white pawn promotion : "<<square_to_coordinate[source_square]<<square_to_coordinate[target_square]<<'N'<<endl;
                            }
                            else
                            {
                                //1 square ahead
                                cout<<" white pawn : "<<square_to_coordinate[source_square]<<square_to_coordinate[target_square]<<endl;
                                //2 square ahead
                                if((source_square >= a2 && source_square <= h2) && !get_bit(occupancy_bitboards[BOTH], target_square-8))
                                {
                                    cout<<" double white pawn : "<<square_to_coordinate[source_square]<<square_to_coordinate[target_square-8]<<endl;
                                }
                            }
                        }
                        //pawn captures
                        attacks = pawn_attacks[side][source_square] & occupancy_bitboards[BLACK];
                        while (attacks)
                        {
                            target_square = get_1st_bit_index(attacks);
                            if(source_square >= a7 && source_square <= h7)
                            {
                                cout<<" white pawn capture promotion : "<<square_to_coordinate[source_square]<<square_to_coordinate[target_square]<<'Q'<<endl;
                                cout<<" white pawn capture promotion : "<<square_to_coordinate[source_square]<<square_to_coordinate[target_square]<<'R'<<endl;
                                cout<<" white pawn capture promotion : "<<square_to_coordinate[source_square]<<square_to_coordinate[target_square]<<'B'<<endl;
                                cout<<" white pawn capture promotion : "<<square_to_coordinate[source_square]<<square_to_coordinate[target_square]<<'N'<<endl;
                            }
                            else
                            {
                                //1 square ahead
                                cout<<" white pawn capture : "<<square_to_coordinate[source_square]<<square_to_coordinate[target_square]<<endl;
                            }
                            delete_bit(attacks, target_square);
                        }
                        if(enpassant != no_sq)
                        {
                            U64 enpassant_attacks = pawn_attacks[side][source_square] & (1ULL << enpassant);
                            if(enpassant_attacks)
                            {
                               int target_enpassant = get_1st_bit_index(enpassant_attacks); 
                               cout<<" white pawn enpassant : "<<square_to_coordinate[source_square]<<square_to_coordinate[target_enpassant]<<endl;
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
                                cout<<" white king side castling is legal : e1g1"<<endl;
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
                                cout<<" white queen side castling is legal : e1c1"<<endl;
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
                        //cout<<" white pawn : "<<square_to_coordinate[source_square]<<endl;
                        target_square = source_square + 8;
                        //quite pawn moves
                        if(!(target_square > h1) && !get_bit(occupancy_bitboards[BOTH], target_square))
                        {
                            //promotion
                            if(source_square >= a2 && source_square <= h2)
                            {
                                cout<<" black pawn promotion : "<<square_to_coordinate[source_square]<<square_to_coordinate[target_square]<<'Q'<<endl;
                                cout<<" black pawn promotion : "<<square_to_coordinate[source_square]<<square_to_coordinate[target_square]<<'R'<<endl;
                                cout<<" black pawn promotion : "<<square_to_coordinate[source_square]<<square_to_coordinate[target_square]<<'B'<<endl;
                                cout<<" black pawn promotion : "<<square_to_coordinate[source_square]<<square_to_coordinate[target_square]<<'N'<<endl;
                            }
                            else
                            {
                                //1 square ahead
                                cout<<" black pawn : "<<square_to_coordinate[source_square]<<square_to_coordinate[target_square]<<endl;
                                //2 square ahead
                                if((source_square >= a7 && source_square <= h7) && !get_bit(occupancy_bitboards[BOTH], target_square+8))
                                {
                                    cout<<" black pawn : "<<square_to_coordinate[source_square]<<square_to_coordinate[target_square+8]<<endl;
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
                                cout<<" black pawn capture promotion : "<<square_to_coordinate[source_square]<<square_to_coordinate[target_square]<<'Q'<<endl;
                                cout<<" black pawn capture promotion : "<<square_to_coordinate[source_square]<<square_to_coordinate[target_square]<<'R'<<endl;
                                cout<<" black pawn capture promotion : "<<square_to_coordinate[source_square]<<square_to_coordinate[target_square]<<'B'<<endl;
                                cout<<" black pawn capture promotion : "<<square_to_coordinate[source_square]<<square_to_coordinate[target_square]<<'N'<<endl;
                            }
                            else
                            {
                                //1 square ahead
                                cout<<" black pawn capture : "<<square_to_coordinate[source_square]<<square_to_coordinate[target_square]<<endl;
                            }
                            delete_bit(attacks, target_square);
                        }
                        if(enpassant != no_sq)
                        {
                            U64 enpassant_attacks = pawn_attacks[side][source_square] & (1ULL << enpassant);
                            if(enpassant_attacks)
                            {
                               int target_enpassant = get_1st_bit_index(enpassant_attacks); 
                               cout<<" black pawn enpassant : "<<square_to_coordinate[source_square]<<square_to_coordinate[target_enpassant]<<endl;
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
                                cout<<" black king side castling is legal : e8g8"<<endl;
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
                                cout<<" black queen side castling is legal : e8c8"<<endl;
                            }
                        }
                    }
                }
            }
            //knigt moves
            if((side == WHITE)?piece == N: piece == n)
            {
                while(bitboard)
                {
                    source_square = get_1st_bit_index(bitboard);
                    attacks = knight_attacks[source_square] & ((side == WHITE)? ~occupancy_bitboards[WHITE]: ~occupancy_bitboards[BLACK]);
                    while (attacks)
                    {
                        target_square = get_1st_bit_index(attacks);
                        //quiet move
                        if(!get_bit(((side == WHITE)?occupancy_bitboards[BLACK]:occupancy_bitboards[WHITE]),target_square))
                        {
                            cout<<" knight quite : "<<square_to_coordinate[source_square]<<square_to_coordinate[target_square]<<endl;
                        }
                        //capture move
                        else
                        {
                            cout<<" knight capture : "<<square_to_coordinate[source_square]<<square_to_coordinate[target_square]<<endl;
                        }
                        //cout<< " beyaz at : "<<target_square<<endl;
                        delete_bit(attacks, target_square);    
                    }
                    delete_bit(bitboard, source_square);    
                }
            }
            //bishop moves
            if((side == WHITE)?piece == B: piece == b)
            {
                while(bitboard)
                {
                    source_square = get_1st_bit_index(bitboard);
                    attacks = get_bishop_attacks(source_square, occupancy_bitboards[BOTH]) & ((side == WHITE)? ~occupancy_bitboards[WHITE]: ~occupancy_bitboards[BLACK]);
                    while (attacks)
                    {
                        target_square = get_1st_bit_index(attacks);
                        //quiet move
                        if(!get_bit(((side == WHITE)?occupancy_bitboards[BLACK]:occupancy_bitboards[WHITE]),target_square))
                        {
                            cout<<" bishop quite : "<<square_to_coordinate[source_square]<<square_to_coordinate[target_square]<<endl;
                        }
                        //capture move
                        else
                        {
                            cout<<" bishop capture : "<<square_to_coordinate[source_square]<<square_to_coordinate[target_square]<<endl;
                        }
                        //cout<< " beyaz at : "<<target_square<<endl;
                        delete_bit(attacks, target_square);    
                    }
                    delete_bit(bitboard, source_square);    
                }
            }
            //rook moves
            if((side == WHITE)?piece == R: piece == r)
            {
                while(bitboard)
                {
                    source_square = get_1st_bit_index(bitboard);
                    attacks = get_rook_attacks(source_square, occupancy_bitboards[BOTH]) & ((side == WHITE)? ~occupancy_bitboards[WHITE]: ~occupancy_bitboards[BLACK]);
                    while (attacks)
                    {
                        target_square = get_1st_bit_index(attacks);
                        //quiet move
                        if(!get_bit(((side == WHITE)?occupancy_bitboards[BLACK]:occupancy_bitboards[WHITE]),target_square))
                        {
                            cout<<" rook quite : "<<square_to_coordinate[source_square]<<square_to_coordinate[target_square]<<endl;
                        }
                        //capture move
                        else
                        {
                            cout<<" rook capture : "<<square_to_coordinate[source_square]<<square_to_coordinate[target_square]<<endl;
                        }
                        //cout<< " beyaz at : "<<target_square<<endl;
                        delete_bit(attacks, target_square);    
                    }
                    delete_bit(bitboard, source_square);    
                }
            }
            //queen moves
            if((side == WHITE)?piece == Q: piece == q)
            {
                while(bitboard)
                {
                    source_square = get_1st_bit_index(bitboard);
                    attacks = get_queen_attacks(source_square, occupancy_bitboards[BOTH]) & ((side == WHITE)? ~occupancy_bitboards[WHITE]: ~occupancy_bitboards[BLACK]);
                    while (attacks)
                    {
                        target_square = get_1st_bit_index(attacks);
                        //quiet move
                        if(!get_bit(((side == WHITE)?occupancy_bitboards[BLACK]:occupancy_bitboards[WHITE]),target_square))
                        {
                            cout<<" queen quite : "<<square_to_coordinate[source_square]<<square_to_coordinate[target_square]<<endl;
                        }
                        //capture move
                        else
                        {
                            cout<<" queen capture : "<<square_to_coordinate[source_square]<<square_to_coordinate[target_square]<<endl;
                        }
                        //cout<< " beyaz at : "<<target_square<<endl;
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
                            cout<<" bishop quite : "<<square_to_coordinate[source_square]<<square_to_coordinate[target_square]<<endl;
                        }
                        //capture move
                        else
                        {
                            cout<<" bishop capture : "<<square_to_coordinate[source_square]<<square_to_coordinate[target_square]<<endl;
                        }
                        //cout<< " beyaz at : "<<target_square<<endl;
                        delete_bit(attacks, target_square);    
                    }
                    delete_bit(bitboard, source_square);    
                }
            }
        }
    }
}
#endif