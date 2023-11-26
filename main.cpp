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

int best_engine_move;
moves engine_moves[1];
moves tension_moves[1];

void parse_fen(const char* fen);
int find_best_move(moves* move_list, int depth);
int parse_move(char* move_string);
void print_top_infos();
void clear_screen();
moves find_deeper_best_moves(const moves& evaluatedMoves);
void computer_is_black();
void computer_is_white();///will write soon

int main()
{
    init_all();
    parse_fen(start_position);
    /*print_top_infos();
    print_chess_board();*/
    computer_is_black();
    //int depth = 5;
    //*engine_moves = search_test(depth, NEGATIVEINFINITY, POSITIVEINFINITY);
    //print_move_list(engine_moves);
    /*cout<<" Best move: ";
    print_move(find_best_move(engine_moves, depth+2));*/
    //cout<<endl<<"____________________"<<endl;
    //*tension_moves = tension_search_test(engine_moves, depth, NEGATIVEINFINITY, POSITIVEINFINITY);
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
int find_best_move(moves* moves_list, int depth) {
    int count = moves_list->count;
    moves* evaluated_move_list = (moves*)malloc(count * sizeof(moves));
    evaluated_move_list->count = 0;
    for (int i = 0; i < count; i++) {
        if (moves_list->move_legality[i] == true)
        {
            add_move_to_evaluated(evaluated_move_list, moves_list->moves[i], moves_list->move_score[i]);
        }
    }
    moves deeper_best_move = find_deeper_best_moves(*evaluated_move_list);
    moves result_list = deep_search_test(&deeper_best_move, depth, NEGATIVEINFINITY, POSITIVEINFINITY);
    sort_move_list_according_to_evaluate_and_guess(&result_list);
    /*cout<<" RESULT LIST SORTING...";
    print_move_list(&result_list);
    cout<<" __________"<<endl;*/
    /*cout << " New Best Moves: Move        Score "<<endl;
    for (int i = 0; i < new_best_moves.count; ++i) {
        cout<<"                ";
        print_move(new_best_moves.moves[i]);
        cout<<"        "<<new_best_moves.move_score[i]<<endl;
    }*/
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
    return result_list.moves[0];
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
moves find_deeper_best_moves(const moves& evaluatedMoves) {
    moves deeper_best_move;
    deeper_best_move.count = 0;
    deeper_best_move.move_score[0] = (side == WHITE) ? INT_MIN : INT_MAX;
    for (int i = 0; i < evaluatedMoves.count; ++i) {
        if (side == WHITE) {
            if (evaluatedMoves.move_score[i] > deeper_best_move.move_score[0]) {
                deeper_best_move.count = 1;
                deeper_best_move.moves[0] = evaluatedMoves.moves[i];
                deeper_best_move.move_score[0] = evaluatedMoves.move_score[i];
                deeper_best_move.move_legality[0] = true;
            }
            else if (evaluatedMoves.move_score[i] == deeper_best_move.move_score[0]) {
                deeper_best_move.moves[deeper_best_move.count] = evaluatedMoves.moves[i];
                deeper_best_move.move_score[deeper_best_move.count] = evaluatedMoves.move_score[i];
                deeper_best_move.move_legality[deeper_best_move.count] = true;
                deeper_best_move.count++;
            }
        }
        else {
            if (evaluatedMoves.move_score[i] < deeper_best_move.move_score[0]) {
                deeper_best_move.count = 1;
                deeper_best_move.moves[0] = evaluatedMoves.moves[i];
                deeper_best_move.move_score[0] = evaluatedMoves.move_score[i];
                deeper_best_move.move_legality[0] = true;
            }
            else if (evaluatedMoves.move_score[i] == deeper_best_move.move_score[0]) {
                deeper_best_move.moves[deeper_best_move.count] = evaluatedMoves.moves[i];
                deeper_best_move.move_score[deeper_best_move.count] = evaluatedMoves.move_score[i];
                deeper_best_move.move_legality[deeper_best_move.count] = true;
                deeper_best_move.count++;
            }
        }
    
    }
    /*std::cout << "Best Moves: "<<endl;
        for (int i = 0; i < bestMove.count; ++i) {
            cout<<" move: "; print_move(bestMove.moves[i]);
            cout<<" | score: "<<bestMove.move_score[i]<<endl;
        }
        cout<<"______________________"<<endl;*/
    return deeper_best_move;
}
void print_top_infos() {
    cout << endl << " Current Evaluation: " << evaluate() << " | Current Tension: " << tension() << endl;
}
void clear_screen() {
    system("cls");
}
void computer_is_black(){
    while (true)
    {
        char* move = new char[MAX_MOVE_LENGTH];   
        print_top_infos();
        print_chess_board();
        cout << endl << " Enter your move (e.g., e2e4): ";
        cin.getline(move, MAX_MOVE_LENGTH);
        int parsed_move = parse_move(move);
        if (!parsed_move || !make_move(parsed_move, all_moves))
        {
            clear_screen();
            cout << endl << " Illegal move, please do a legal move or write it in correct format. (Ex. e2e4, a7a8q...)" << endl;
            continue;
        }
        *engine_moves = search_test(6, NEGATIVEINFINITY, POSITIVEINFINITY);
        best_engine_move = find_best_move(engine_moves, 7);
        clear_screen();
        cout << " Batu is playing: ";
        print_move(best_engine_move);
        cout << endl;
        make_move(best_engine_move, all_moves);
    }
}