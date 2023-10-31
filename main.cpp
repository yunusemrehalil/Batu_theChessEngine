#include <iostream>
#include <chrono>

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

/*
    binary move bits representation                                 hexadecimal constants
    
    0000 0000 0000 0000 0011 1111   source square                   0x3f
    0000 0000 0000 1111 1100 0000   targetsquare                    0xfc0
    0000 0000 1111 0000 0000 0000   piece                           0xf000
    0000 1111 0000 0000 0000 0000   promoted piece                  0xf0000
    0001 0000 0000 0000 0000 0000   capture flag                    0x100000
    0010 0000 0000 0000 0000 0000   double pawn push flag           0x200000
    0100 0000 0000 0000 0000 0000   enpassant flag                  0x400000
    1000 0000 0000 0000 0000 0000   castling flag                   0x800000
*/

/*
                                        castling    move        binary  decimal
                                        right       update      
        king & rook did not move:       1111    &   1111    =   1111    15
                white king moved:       1111    &   1100    =   1100    12
     white king's rook not moved:       1111    &   1110    =   1110    14
   white queens's rook not moved:       1111    &   1101    =   1101    13
   //////////////////////////////
                black king moved:       1111    &   0011    =   0011    3
     black king's rook not moved:       1111    &   1011    =   1011    11
   black queens's rook not moved:       1111    &   0111    =   0111    7
*/


long nodes;
long cummulative_nodes;
long old_nodes; 
static inline void perft_driver(int depth)
{
    if(depth == 0)
    {
        nodes++;
        return;
    }
    moves move_list[1];
    generate_moves(move_list);
    for (int i = 0; i < move_list->count; i++)
    {
        //init move
        int move = move_list->moves[i];
        //preserve board state
        copy_board();
        //make move
        if(!make_move(move, all_moves))
        {
            continue;
        }
        perft_driver(depth-1);
        take_back();

    }
}

void print_move_list(moves *move_list);
void print_move(int move);
void print_bitboard(U64 bitboard);
void print_chess_board();
void parse_fen(const char *fen);

void perft_test(int depth)
{   
    cout<<endl<<" PERFORMANCE TEST"<<endl<<endl;
    moves move_list[1];
    generate_moves(move_list);
    auto startTime = chrono::high_resolution_clock::now();
    for (int i = 0; i < move_list->count; i++)
    {
        int move = move_list->moves[i];
        copy_board();
        if(!make_move(move, all_moves))
        {
            continue;
        }
        cummulative_nodes = nodes;
        perft_driver(depth-1);
        old_nodes = nodes - cummulative_nodes;
        take_back();
        cout<<" move: "<<square_to_coordinate[get_move_source(move)]<<square_to_coordinate[get_move_target(move)]<<promoted_piece[get_move_promoted(move)]<<
                    " | nodes: "<<old_nodes<<endl;
    }
    auto endTime = chrono::high_resolution_clock::now();
    chrono::milliseconds duration = chrono::duration_cast<chrono::milliseconds>(endTime - startTime);
    cout<<endl<<endl<<" Depth: "<<depth<<endl;
    cout<<" Nodes: "<<nodes<<endl;
    cout << " Loop took " << duration.count() << " milliseconds" << endl;
}
int main()
{
    
    init_all();
    parse_fen(trying_position);
    print_chess_board();
    
    //print_move_list(move_list);
    //auto startTime = chrono::high_resolution_clock::now();
    perft_test(5);
    /*auto endTime = chrono::high_resolution_clock::now();
    chrono::microseconds duration = chrono::duration_cast<chrono::milliseconds>(endTime - startTime);*/
    /*cout << " Loop took " << duration.count() << " microseconds" << endl;
    cout << " Nodes : " << nodes << endl;*/
    
    /*//preserve board state
    copy_board();
    //parse fen
    parse_fen(empty_board);
    print_chess_board();
    //restore board state
    take_back();
    print_chess_board();*/
    /*moves move_list[1];
    move_list->count = 0;
    add_move(move_list, encode_move(d7, e8, P, B, 1, 0, 0, 0));
    //int move = encode_move(e7, d8, P, Q, 1, 0, 0, 0);
    generate_moves(move_list);
    print_move_list(move_list);*/
    /*int move = encode_move(e7, d8, P, Q, 1, 0, 0, 0);
    int source_square = get_move_source(move);
    cout<<" source square : "<<square_to_coordinate[source_square]<<endl;
    int target_square = get_move_target(move);
    cout<<" target square : "<<square_to_coordinate[target_square]<<endl;
    int piece = get_move_piece(move);
    cout<<" piece : "<<ascii_pieces[piece]<<endl;
    int promoted = get_move_promoted(move);
    cout<<" promoted : "<<ascii_pieces[promoted]<<endl;
    int captured = get_move_capture(move);
    cout<<" captured : "<<((captured)?"captured":"non-captured")<<endl;
    int doublepawn = get_move_doublepawn(move);
    cout<<" double pawn : "<<((doublepawn)?"double pawn":"non-double pawn")<<endl;
    int enpassant = get_move_enpassant(move);
    cout<<" double pawn : "<<((enpassant)?"enpassant":"non-enpassant")<<endl;
    int castling = get_move_castling(move);
    cout<<" double pawn : "<<((castling)?"castling":"non-castling")<<endl;*/
    /*auto startTime = chrono::high_resolution_clock::now();
    for(int i=0; i<10000000; i++)
    {
        generate_moves();
    }
    auto endTime = chrono::high_resolution_clock::now();
    chrono::microseconds duration = chrono::duration_cast<chrono::microseconds>(endTime - startTime);
    cout << " Loop took " << duration.count() << " microseconds" << endl;*/


    //print_bitboard(occupancy_bitboards[BOTH]);
    /*print_bitboard(pawn_attacks[WHITE][e4]&piece_bitboards[p] );
    cout<<"is e4 attacked by black pawn? "<<((pawn_attacks[WHITE][e4]&piece_bitboards[p])?"yes":"no");*/
    //print_attacked_squares(WHITE);
    cin.get();
    return 0;

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
    /*side = WHITE;
    castle = wk | wq | bk | bq;*/
    //print_chess_board();
    /*cout<<ascii_pieces[P]<<endl;
    cout<<ascii_pieces[char_pieces['K']];*/
    //print_bitboard(occupancy);
    /*U64 occupancy = 0ULL;
    set_bit(occupancy, e5);
    set_bit(occupancy, f5);
    set_bit(occupancy, c4);*/
    /*set_bit(occupancy, c2);
    set_bit(occupancy, e3);
    set_bit(occupancy, g4);
    set_bit(occupancy, c6);
    set_bit(occupancy, g2);*/
    //print_bitboard(get_queen_attacks(e4, occupancy));
    //print_bitboard(piece_bitboards[r]);
    
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
    cout<<endl<<" move    piece    capture    doublepawn    enpassant    castling"<<endl;
    for(int move_count=0; move_count<move_list->count; move_count++)
    {
        int move = move_list->moves[move_count];
        cout<<' '<<square_to_coordinate[get_move_source(move)]<<square_to_coordinate[get_move_target(move)]<<(get_move_promoted(move)?promoted_piece[get_move_promoted(move)]:' ')<<"   "<<
                        ascii_pieces[get_move_piece(move)]
                        <<"        "<<(get_move_capture(move)?1:0)
                        <<"          "<<(get_move_doublepawn(move)?1:0)
                        <<"             "<<(get_move_enpassant(move)?1:0)
                        <<"            "<<(get_move_castling(move)?1:0)<<endl;
    }
    cout<<" Total number of moves : "<<move_list->count<<endl;
}









