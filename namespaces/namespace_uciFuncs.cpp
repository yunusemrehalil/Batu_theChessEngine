#pragma once
#ifndef UCIFUNCS_CPP
#define UCIFUNCS_CPP

#include <cstring>
#include "../headers/constants.hpp"
#include "../headers/enums.hpp"
#include "../headers/arrays.hpp"
#include "namespace_staticFuncs.cpp"
#include "namespace_printFuncs.cpp"
#include "namespace_testFuncs.cpp"



using namespace std;
using namespace printFuncs;
using namespace sif;
using namespace testFuncs;

namespace uciFuncs{
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
        return deeper_best_move;
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
        return result_list.moves[0];
    }    
    static inline void search_position(int depth){
        moves engine_moves[1];
        *engine_moves = search_test(depth, NEGATIVEINFINITY, POSITIVEINFINITY);
        int best_move = find_best_move(engine_moves, depth);
        cout<<"bestmove ";
        print_move(best_move);
    }
    static inline void parse_fen(const char* fen) {
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
    static inline int parse_move(char* move_string)
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
    static inline void parse_position(char *command){
        command += 9;
        char *current_char = command;
        if(strncmp(command, "startpos", 8) == 0) parse_fen(start_position);
        else
        {
            current_char = strstr(command, "fen");
            if(current_char == NULL)
            {
                parse_fen(start_position);
            }
            else
            {
                current_char += 4;
                parse_fen(current_char);
            }
        }
        current_char = strstr(command, "moves");
        if(current_char != NULL)
        {
            current_char += 6;
            while(*current_char)
            {
                int move = parse_move(current_char);
                if(move == 0)
                {
                    break;
                }
                make_move(move, all_moves);
                while(*current_char && *current_char != ' ') current_char++;
                current_char++;
            }
        }
        print_chess_board();
    }
    static inline void parse_go(char *command){
        int depth = -1;
        char *current_depth = NULL;
        if(current_depth = strstr(command, "depth"))
        {
            depth = atoi(current_depth + 6);
        }
        else
        {
            depth = 6;
        }
        search_position(depth);
    }
    static inline void uci_loop(){

        char input[2000];
        
        /*setvbuf(stdin, input, _IOFBF, sizeof(input));
        setvbuf(stdout, input, _IOFBF, sizeof(input));*/
        cout<<"id name batu"<<endl<<"id author Yunus Emre Halil"<<endl<<"uciok"<<endl;

        while(1)
        {
            memset(input, 0, sizeof(input));
            fflush(stdout);
            if(!fgets(input, 2000, stdin))
            {
                continue;
            }
            else if(input[0] == '\n')
            {
                continue;
            }
            else if(strncmp(input, "isready", 7) == 0)
            {
                cout<<"readyok"<<endl;
                continue;
            }
            else if(strncmp(input, "position", 8) == 0)
            {
                parse_position(input);
            }
            else if(strncmp(input, "ucinewgame", 10) == 0)
            {
                parse_position("position startpos");
            }
            else if(strncmp(input, "go", 2) == 0)
            {
                parse_go(input);
            }            
            else if(strncmp(input, "quit", 4) == 0)
            {
                break;
            }
            else if(strncmp(input, "uci", 3) == 0)
            {
                cout<<"id name batu"<<endl<<"id author Yunus Emre Halil"<<endl<<"uciok"<<endl;
            }
        }
    }
    
}

#endif