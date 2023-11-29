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
#include "namespaces/namespace_uciFuncs.cpp"

using namespace std;
using namespace sif;
using namespace maskPieceFuncs;
using namespace initializeFuncs;
using namespace magicNumberFuncs;
using namespace testFuncs;
using namespace printFuncs;
using namespace uciFuncs;

int best_engine_move;
moves engine_moves[1];
moves tension_moves[1];

void print_top_infos();
void clear_screen();
void computer_is_black();
void computer_is_white();///will write soon

int main()
{
    init_all();
    //parse_fen(start_position);
    uci_loop();
    /*parse_position("position startpos moves d2d4 d7d5");
    print_chess_board();
    parse_go("go depth 8");*/
    /*print_top_infos();
    print_chess_board();*/
    //computer_is_black();
    //int depth = 5;
    //*engine_moves = search_test(depth, NEGATIVEINFINITY, POSITIVEINFINITY);
    //print_move_list(engine_moves);
    /*cout<<" Best move: ";
    print_move(find_best_move(engine_moves, depth+2));*/
    //cout<<endl<<"____________________"<<endl;
    //*tension_moves = tension_search_test(engine_moves, depth, NEGATIVEINFINITY, POSITIVEINFINITY);
    //cin.get();
    return 0;
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