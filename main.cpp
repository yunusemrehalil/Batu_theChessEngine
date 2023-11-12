#include <iostream>
#include <chrono>
#include <algorithm>

#include "headers/constants.hpp"
#include "headers/magic_numbers.hpp"
#include "headers/enums.hpp"
#include "headers/arrays.hpp"
#include "namespaces/namespace_staticFuncs.cpp"
#include "namespaces/namespace_maskPieceFuncs.cpp"
#include "namespaces/namespace_initializeFuncs.cpp"
#include "namespaces/namespace_magicNumberFuncs.cpp"
#include "namespaces/namespace_testFuncs.cpp"
#include "namespaces/namespace_printFuncs.cpp"

using namespace std;
using namespace sif;
using namespace maskPieceFuncs;
using namespace initializeFuncs;
using namespace magicNumberFuncs;
using namespace testFuncs;
using namespace printFuncs;

void parse_fen(const char* fen);
void order_moves(moves* move_list);
int find_best_move(moves* move_list);
int parse_move(char* move_string);
void print_top_infos();
EvaluatedMove findBestMove(const EvaluatedMove &evaluatedMoves);

int main()
{

    char* move = new char[MAX_MOVE_LENGTH];
    int best_engine_move;
    moves engine_moves[1];
    init_all();
    parse_fen(start_position);
    
    while (true) 
    {
        print_top_infos();
        print_chess_board();
        cout << endl << " Enter your move (e.g., e2e4): ";
        cin.getline(move, MAX_MOVE_LENGTH);
        int parsed_move = parse_move(move);
        if (!parsed_move || !make_move(parsed_move, all_moves)) 
        {
            cout << endl << " Illegal move, please do a legal move or write it in correct format. (Ex. e2e4, a7a8q...)" << endl;
            continue;
        }
        *engine_moves = search_test(5, NEGATIVEINFINITY, POSITIVEINFINITY);
        best_engine_move = find_best_move(engine_moves);
        cout << " Batu is playing: ";
        print_move(best_engine_move);
        cout << endl;
        make_move(best_engine_move, all_moves);
    }
    cin.get();
    return 0;
}

void parse_fen(const char* fen) {
    memset(piece_bitboards, 0ULL, sizeof(piece_bitboards));
    memset(occupancy_bitboards, 0ULL, sizeof(occupancy_bitboards));
    side = 0;
    enpassant = no_sq;
    castle = 0;
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        for (int j = 0; j < BOARD_SIZE; j++)
        {
            int square = i * 8 + j;
            if ((*fen >= 'a' && *fen <= 'z') || (*fen >= 'A' && *fen <= 'Z'))
            {
                int piece = char_pieces[*fen];
                set_bit(piece_bitboards[piece], square);
                fen++;
            }
            if ((*fen >= '0' && *fen <= '9'))
            {
                int offset = *fen - '0';
                int piece = -1;
                for (int bb_count = P; bb_count <= k; bb_count++)
                {
                    if (get_bit(piece_bitboards[bb_count], square)) piece = bb_count;
                }
                if (piece == -1)
                {
                    j--;
                }
                j += offset;
                fen++;
            }
            if (*fen == '/')
            {
                fen++;
            }
        }
    }
    fen++;
    (*fen == 'w') ? (side = WHITE) : side = BLACK;
    fen += 2;
    while (*fen != ' ')
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
    if (*fen != '-')
    {
        int j = fen[0] - 'a';
        int i = 8 - (fen[1] - '0');
        enpassant = i * 8 + j;
    }
    else
    {
        enpassant = no_sq;
    }
    for (int piece = P; piece <= K; piece++)
    {
        occupancy_bitboards[WHITE] |= piece_bitboards[piece];
    }
    for (int piece = p; piece <= k; piece++)
    {
        occupancy_bitboards[BLACK] |= piece_bitboards[piece];
    }
    occupancy_bitboards[BOTH] |= occupancy_bitboards[WHITE];
    occupancy_bitboards[BOTH] |= occupancy_bitboards[BLACK];
}
void order_moves(moves* move_list) {
    for (int i = 0; i < move_list->count; i++)
    {
        int move = move_list->moves[i];
        int move_score_guess = 0;
        int source_piece = get_move_piece(move);
        int target_piece = get_captured_piece(move);
        if (get_move_capture(move))
        {
            move_score_guess = 10 * piece_value[target_piece] - piece_value[source_piece];
            cout << ascii_pieces[source_piece] << ascii_pieces[target_piece] << ": " << move_score_guess << endl;
        }
    }
}
int find_best_move(moves* moves_list) {
    int count = moves_list->count;
    EvaluatedMove* evaluated_move_list = (EvaluatedMove*)malloc(count * sizeof(EvaluatedMove));
    evaluated_move_list->count = 0;
    for (int i = 0; i < count; i++) {
        if(moves_list->move_legality[i] == 1)
        {
            add_move_to_evaluated(evaluated_move_list, moves_list->moves[i], moves_list->move_score[i]);
        }   
    }
    EvaluatedMove bestMove = findBestMove(*evaluated_move_list);

    /*cout << "Best Moves: "<<endl;
    for (int i = 0; i < bestMove.count; ++i) {
        print_move(bestMove.moves[i]);
    }
    cout << endl;
    cout << "Best Score: " << bestMove.move_score[0] << endl;

    // Sort the evaluated_move_list array
    cout<<endl<<" move    score"<<endl;
    for(int move_count=0; move_count<evaluated_move_list->count; move_count++)
    {
        int move = evaluated_move_list->moves[move_count];
        cout<<' '<<square_to_coordinate[get_move_source(move)]<<square_to_coordinate[get_move_target(move)]<<(get_move_promoted(move)?promoted_piece[get_move_promoted(move)]:' ')<<"  "
                    <<" "<<evaluated_move_list->move_score[move_count]<<endl;
    }
    cout<<" Total number of moves : "<<evaluated_move_list->count<<endl;*/
    return bestMove.moves[0];
    /*(side == WHITE)? best_move = best_move_WHITE: best_move = best_move_BLACK;
    cout<<endl<<" Best move for "<<((side==WHITE)?"white: ":"black: ")<<square_to_coordinate[get_move_source(best_move)]
            <<square_to_coordinate[get_move_target(best_move)]<<promoted_piece[get_move_promoted(best_move)]<<endl;*/
    //return *evaluated_move_list;
}
int parse_move(char* move_string)
{
    moves move_list[1];
    generate_moves(move_list);
    int source_square = (move_string[0] - 'a') + (8 - (move_string[1] - '0')) * 8;
    int target_square = (move_string[2] - 'a') + (8 - (move_string[3] - '0')) * 8;

    for (int move_count = 0; move_count < move_list->count; move_count++)
    {
        int move = move_list->moves[move_count];
        if (source_square == get_move_source(move) && target_square == get_move_target(move))
        {
            int promoted_piece = get_move_promoted(move);
            if (promoted_piece)
            {
                if ((promoted_piece == Q || promoted_piece == q) && move_string[4] == 'q')
                    return move;

                else if ((promoted_piece == R || promoted_piece == r) && move_string[4] == 'r')
                    return move;

                else if ((promoted_piece == B || promoted_piece == b) && move_string[4] == 'b')
                    return move;

                else if ((promoted_piece == N || promoted_piece == n) && move_string[4] == 'n')
                    return move;
                continue;
            }
            return move;
        }
    }
    return 0;
}
EvaluatedMove findBestMove(const EvaluatedMove &evaluatedMoves) {
    EvaluatedMove bestMove;
    bestMove.count = 0;
    bestMove.move_score[0] = (side==WHITE) ? INT_MIN : INT_MAX;
    for (int i = 0; i < evaluatedMoves.count; ++i) {
        if (side==WHITE) 
        {
            if (evaluatedMoves.move_score[i] > bestMove.move_score[0]) 
            {
                bestMove.count = 1;
                bestMove.moves[0] = evaluatedMoves.moves[i];
                bestMove.move_score[0] = evaluatedMoves.move_score[i];
            } 
            else if (evaluatedMoves.move_score[i] == bestMove.move_score[0]) 
            {
                bestMove.moves[bestMove.count] = evaluatedMoves.moves[i];
                bestMove.move_score[bestMove.count] = evaluatedMoves.move_score[i];
                bestMove.count++;
            }
        } 
        else 
        {
            if (evaluatedMoves.move_score[i] < bestMove.move_score[0]) {
                bestMove.count = 1;
                bestMove.moves[0] = evaluatedMoves.moves[i];
                bestMove.move_score[0] = evaluatedMoves.move_score[i];
            } else if (evaluatedMoves.move_score[i] == bestMove.move_score[0]) {
                bestMove.moves[bestMove.count] = evaluatedMoves.moves[i];
                bestMove.move_score[bestMove.count] = evaluatedMoves.move_score[i];
                bestMove.count++;
            }
        }
    }
    return bestMove;
}
void print_top_infos(){
    cout<<"Current Evaluation: "<<evaluate()<< " | Current Tension: "<<tension()<<endl;
}


