#pragma once
#ifndef TESTFUNCS_CPP
#define TESTFUNCS_CPP

#include "../headers/constants.hpp"
#include "../headers/arrays.hpp"
#include "namespace_staticFuncs.cpp"
#include "namespace_printFuncs.cpp"


using namespace std;
using namespace printFuncs;

namespace testFuncs{
    static inline int evaluate()
    {
        U64 material = 0;
        int evaluation = 0, piece, square;
        for(int current_piece = P; current_piece <= k; current_piece++)
        {
            material = piece_bitboards[current_piece];
            while(material)
            {
                piece = current_piece;
                square = get_1st_bit_index(material);
                evaluation += piece_value[piece];
                delete_bit(material, square);
            }
        }
        return (side==WHITE)?evaluation:-evaluation;
}
    static inline int tension()
    {
        moves move_list[1];
        generate_moves(move_list);
        int capture_count=0, checking_count=0, promotion_count=0;
        for(int i = 0; i<move_list->count; i++)
        {
            if(get_move_capture(move_list->moves[i]))
            {
                capture_count++;
            }
            if(get_move_checking(move_list->moves[i]))
            {
                checking_count++;
            }
            if(get_move_promoted(move_list->moves[i]))
            {
                promotion_count++;
            }
        }
        return capture_count + checking_count + promotion_count;
    }
    static inline void order_moves(moves* move_list) {
        for (int i = 0; i < move_list->count; i++)
        {
            move_list->move_score_guess[i] = 0;
            int move = move_list->moves[i];
            int source_piece = get_move_piece(move);
            int target_piece = get_captured_piece(move);
            int target_square = get_move_target(move);
            int promoted_piece = get_move_promoted(move);
            int is_check = get_move_checking(move);
            if (get_move_capture(move))
            {
                move_list->move_score_guess[i] =-(10 * piece_value[target_piece] - piece_value[source_piece]);
            }
            if(get_move_promoted(move))
            {
                move_list->move_score_guess[i] += -(piece_value[promoted_piece]);
            }
            if(target_square == d4 || target_square == d5 || target_square == e4 || target_square == e5)
            {
                move_list->move_score_guess[i] += (piece_value[source_piece]);
            }
            if((source_piece == B && (target_square == b2 || target_square == g2)) ||
               (source_piece == b && (target_square == b7 || target_square == g7)) ||
               (source_piece == N && (target_square == c3 || target_square == f3)) ||
               (source_piece == n && (target_square == c6 || target_square == f6)))
            {
                move_list->move_score_guess[i] += piece_value[source_piece];
            }
            if(is_check)
            {
                move_list->move_score_guess[i] += piece_value[source_piece]/10;
            }
        }
    }
    static inline void sort_move_list_according_to_score_guess(moves* moveList) {
        order_moves(moveList);
        if(side==WHITE)
        {
            for (int i = 0; i < moveList->count - 1; ++i) 
            {
                for (int j = 0; j < moveList->count - i - 1; ++j) {
                    if (moveList->move_score_guess[j] < moveList->move_score_guess[j + 1]) {
                        swap(moveList->moves[j], moveList->moves[j + 1]);
                        swap(moveList->move_score[j], moveList->move_score[j + 1]);
                        swap(moveList->move_tension[j], moveList->move_tension[j + 1]);
                        swap(moveList->move_legality[j], moveList->move_legality[j + 1]);
                        swap(moveList->move_score_guess[j], moveList->move_score_guess[j + 1]);
                    }
                }
            }
        }
        else
        {
            for (int i = 0; i < moveList->count - 1; ++i) 
            {
                for (int j = 0; j < moveList->count - i - 1; ++j) {
                    if (moveList->move_score_guess[j] > moveList->move_score_guess[j + 1]) {
                        swap(moveList->moves[j], moveList->moves[j + 1]);
                        swap(moveList->move_score[j], moveList->move_score[j + 1]);
                        swap(moveList->move_tension[j], moveList->move_tension[j + 1]);
                        swap(moveList->move_legality[j], moveList->move_legality[j + 1]);
                        swap(moveList->move_score_guess[j], moveList->move_score_guess[j + 1]);
                    }
                }
            }
        }  
}

    /*############            PERFT TEST FUNCTIONS            ############*/
    static inline void perft_driver(int depth)
    {
        if(depth == 0)
        {
            nodes++;
            return;
        }
        moves move_list[1];
        generate_moves(move_list);
        sort_move_list(move_list);
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
    static inline void perft_test(int depth)
    {   
        cout<<endl<<" PERFORMANCE TEST"<<endl<<endl;
        moves move_list[1];
        generate_moves(move_list);
        auto startTime = chrono::high_resolution_clock::now();
        for (int i = 0; i < move_list->count; i++)
        {
            sort_move_list(move_list);
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
    /*############            SEARCH TEST FUNCTIONS            ############*/
    static inline int search_driver(int depth, int alpha, int beta){
        int evaluation;
        if(depth == 0)
        {
            nodes++;
            return evaluate();
        }
        moves move_list[1];
        generate_moves(move_list);
        sort_move_list_according_to_score_guess(move_list);
        if(move_list->count == 0)
        {
            cout<<"No move here..."<<endl;
            return 0;
        }
        for(int i = 0; i<move_list->count; i++)
        {
            int move = move_list->moves[i];
            copy_board();
            if(!make_move(move, all_moves))
            {
                continue;
            }
            nodes++;
            evaluation = -search_driver(depth-1, -beta, -alpha);
            take_back();
            if(evaluation >= beta)
            {
                return beta;
            }
            alpha = max(alpha, evaluation);
        }
        return alpha;
    }
    static inline moves search_test(int depth, int alpha, int beta){
        //cout<<endl<<" SEARCH TEST  | Current Evaluation : "<<evaluate()<<endl<<endl;
        int best;
        moves move_list[1];
        generate_moves(move_list);
        sort_move_list_according_to_score_guess(move_list);
        //auto startTime = chrono::high_resolution_clock::now();
        for (int i = 0; i < move_list->count; i++)
        {
            int move = move_list->moves[i];
            copy_board();
            if(!make_move(move, all_moves))
            {
                move_list->move_legality[i] = false;
                continue;
            }
            move_list->move_legality[i] = true;
            cummulative_nodes = nodes;
            best = -search_driver(depth-1, -beta, -alpha);
            old_nodes = nodes - cummulative_nodes;
            take_back();
            move_list->move_score[i] = ((side==WHITE)?best:-best);
            /*cout<<" move: "<<square_to_coordinate[get_move_source(move)]<<square_to_coordinate[get_move_target(move)]<<promoted_piece[get_move_promoted(move)]<<
                        " | depth: "<<depth<<" evaluated: "<<((side==WHITE)?best:-best)<<" nodes: "<<old_nodes
                        << " | score_guess: "<< move_list->move_score_guess[i]<<endl;*/
        }
        /*auto endTime = chrono::high_resolution_clock::now();
        chrono::milliseconds duration = chrono::duration_cast<chrono::milliseconds>(endTime - startTime);
        cout << " Loop took " << duration.count() << " milliseconds" << endl;*/
        return *move_list;
    }
    /*############            DEEP SEARCH TEST FUNCTIONS            ############*/
    static inline int deep_search_driver(int depth, int alpha, int beta){
        int evaluation;
        if(depth == 0)
        {
            nodes++;
            return evaluate();
        }
        if(depth>=3)
        {
            copy_board();
            side ^= 1;
            enpassant = no_sq;
            int score = -deep_search_driver(depth-1-2, -beta, -beta+1);
            take_back();
            if(score >= beta)
            {
                return beta;
            }
        }
        moves move_list[1];
        generate_moves(move_list);
        sort_move_list_according_to_score_guess(move_list);
        if(move_list->count == 0)
        {
            cout<<"No move here..."<<endl;
            return 0;
        }
        for(int i = 0; i<move_list->count; i++)
        {
            int move = move_list->moves[i];
            copy_board();
            if(!make_move(move, all_moves))
            {
                continue;
            }
            //if(tension() == 0) continue;
            evaluation = -deep_search_driver(depth-1, -beta, -alpha);
            take_back();
            if(evaluation >= beta)
            {
                return beta;
            }
            alpha = max(alpha, evaluation);
        }
        return alpha;
    }    
    static inline moves deep_search_test(moves *move_list, int depth, int alpha, int beta){
        sort_move_list_according_to_score_guess(move_list);
        int best;
        for (int i = 0; i < move_list->count; i++)
        {
            int move = move_list->moves[i];
            copy_board();
            if(!make_move(move, all_moves))
            {
                continue;
            }
            best = -deep_search_driver(depth-1, -beta, -alpha);
            take_back();
            move_list->move_score[i] = ((side==WHITE)?best:-best);
            /*cout<<" move: "<<square_to_coordinate[get_move_source(move)]<<square_to_coordinate[get_move_target(move)]<<promoted_piece[get_move_promoted(move)]<<
                        " | depth: "<<depth<<" evaluated: "<<((side==WHITE)?best:-best)
                            <<" | score guess: "<< move_list->move_score_guess[i]<<endl;*/
            /*cout<<" Move: "; print_move(move);
            cout<<" | Score: "<<move_list->move_score[i]<<endl;*/
        }
        return *move_list;
    }
    /*############            TENSION SEARCH TEST FUNCTIONS            ############*/
    static inline int tension_search_driver(int depth, int alpha, int beta){
        int tension_count;
        if(depth == 0)
        {
            nodes++;
            return tension();
        }
        moves move_list[1];
        generate_moves(move_list);
        if(move_list->count == 0)
        {
            cout<<"No move here..."<<endl;
            return 0;
        }
        for(int i = 0; i<move_list->count; i++)
        {
            int move = move_list->moves[i];
            copy_board();
            if(!make_move(move, all_moves))
            {
                continue;
            }
            tension_count = -tension_search_driver(depth-1, -beta, -alpha);
            //bestEvaluation = max(evaluation, bestEvaluation);
            take_back();
            if(tension() >= beta)
            {
                return beta;
            }
            alpha = max(alpha, tension_count);
        }
        return alpha;
        //return bestEvaluation;
    }
    static inline moves tension_search_test(moves *move_list, int depth, int alpha, int beta){
        int best;
        cout<<endl<<"Evaluated Tensions : "<<endl<<"__________"<<endl;
        //auto startTime = chrono::high_resolution_clock::now();
        for (int i = 0; i < move_list->count; i++)
        {
            //sort_move_list(move_list);
            int move = move_list->moves[i];
            copy_board();
            if(!make_move(move, all_moves))
            {
                continue;
            }
            best = -tension_search_driver(depth-1, -beta, -alpha);
            take_back();
            move_list->move_tension[i] = ((side==WHITE)?best:-best);
            cout<<" move: "<<square_to_coordinate[get_move_source(move)]<<square_to_coordinate[get_move_target(move)]<<promoted_piece[get_move_promoted(move)]<<
                        " | depth: "<<depth<<" tension: "<<((side==WHITE)?best:-best)<<endl;
        }
        /*auto endTime = chrono::high_resolution_clock::now();
        chrono::milliseconds duration = chrono::duration_cast<chrono::milliseconds>(endTime - startTime);*/
        //cout << " Loop took " << duration.count() << " milliseconds" << endl;
        return *move_list;
    }
}

#endif