#include <chrono>
#include <stdio.h>
#include <string>
#include <iostream>
#include <sstream>

#include "search.h"
#include "utils/types.h"
#include "test/perft.h"

//TODO: 


int main( int argc, char *argv[] )
{
    std::string initial_fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    std::string random_fen = "5b2/4p1P1/7p/7q/3k1K2/2p1p1P1/b4Prr/2n1B3 w - - 0 1";
    std::string castle_fen = "rnbqkbnr/pp1pp1pp/2p5/4Pp2/2B5/5N2/PPPP1PPP/RNBQK2R w KQkq - 0 1";
    std::string buggy_fen = "2r3k1/1q1nbppp/r3p3/3pP3/pPpP4/P1Q2N2/2RN1PPP/2R4K b - b3 0 23";
    std::string test_special_ep_fen = "8/6bb/8/8/R1pP2k1/4P3/P7/K7 b - d3 0 0";
    std::string kiwipete = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - ";
    std::string in_check = "rnb1kbnr/pppppppp/8/8/1q6/3P4/PPP1PPPP/RNBQKBNR w - - 0 1";
    std::string pos3 = "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - ";
    std::string pos4 = "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1";

    BoardState board;
    board.init(kiwipete);

    bool running = true;
    while(running) {
        std::string cmd;
        size_t param1;
        std::cin >> cmd >> param1;
        std::vector<std::string> tokens;
        std::istringstream iss(cmd);

        if(strcmp(cmd.c_str(), "stop") == 0) {
            running = false;
        }
    
        if(strcmp(cmd.c_str(), "go") == 0) {
            printf("%s %u\n", cmd.c_str(), param1);

            Line pline;
            Search s(1000);

            auto search_start = std::chrono::steady_clock::now();
            s.search(board, param1, &pline);
            auto search_end = std::chrono::steady_clock::now();
            std::chrono::duration<double> elapsed = search_end - search_start;

            printf("Searched %u nodes at depth %u in %fs \n", s.get_nodes_searched(), param1, elapsed);       
            s.print_line(board, pline);
        }

        // Search s(2);
        
        // std::cout << "Evaluating..." << '\n';
        // board.print_squares();

        // auto start = std::chrono::steady_clock::now();
        // s.iterative_search(board);
        // auto end = std::chrono::steady_clock::now();

        // Duration elapsed = end - start; 

        // std::cout << "searched  depth " << s.get_depth_searched() - 1 << " and " << s.get_nodes_searched() << " nodes in " << elapsed.count() << "s" << '\n'; 

    }
    return 0;
}
