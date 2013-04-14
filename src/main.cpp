#include <algorithm>
#include <cassert>
#include <cctype>
#include <chrono>
#include <iostream>
#include <regex>
#include <string>
#include <vector>

// Need Win32 to print pretty colors
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "Board.hpp"
#include "moves.hpp"
#include "ai.hpp"
#include "zobrist.hpp"

using namespace std;

void initBoard(Board& board);
void drawBoard(const Board& board);
bool askForHumansMove(const Board& board, Move& move);
void decideCpusMove(const Board& board, Move& move);

int main()
{
    // Print big numbers with thousands separators
    locale loc("");
    cout.imbue(loc);

    Board board;
    initBoard(board);
    drawBoard(board);
    while(true) {
        // @TODO@ -- accommodate 3 or 4 players
        if(board.countPieces(PLAYER1) == 0) {
            cout << "Red wins!" << endl;
            return 0;
        }

        if(board.countPieces(PLAYER2) == 0) {
            cout << "Blue wins!" << endl;
            return 0;
        }

        // @TODO@ -- accommodate 3 or 4 players
        if(!hasLegalMove(board, PLAYER1) && !hasLegalMove(board, PLAYER2)) {
            int p1_num_pieces = board.countPieces(PLAYER1);
            int p2_num_pieces = board.countPieces(PLAYER2);
            if(p1_num_pieces > p2_num_pieces) {
                cout << "Blue";
            } else if(p1_num_pieces < p2_num_pieces) {
                cout << "Red";
            } else {
                cout << "Nobody";
            }
            cout << " wins!" << endl;
            return 0;
        }

        if(hasLegalMove(board, PLAYER1)) {
            Move move;
            if(!askForHumansMove(board, move)) {
                return 0;
            }
            makeMove(board, move);
            drawBoard(board);
        }

        cout << "AI is thinking..." << endl;

        // CPU's turn
        if(hasLegalMove(board, PLAYER2)) {
            Move move;
            decideCpusMove(board, move);
            makeMove(board, move);
            drawBoard(board);
        }
    }

    return 0;
}

void initBoard(Board& board)
{
    for(int y = 0; y < 7; ++y) {
        for(int x = 0; x < 7; ++x) {
            board(x, y) = EMPTY_SQUARE;
        }
    }
    board(0, 0) = PLAYER1;
    board(6, 0) = PLAYER2;
    board(0, 6) = PLAYER2;
    board(6, 6) = PLAYER1;
}

void drawBoard(const Board& board)
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    cout << "  A B C D E F G" << endl;
    cout << " ---------------" << endl;
    for(int y = 0; y < 7; ++y) {
        cout << (y + 1) << "|";
        for(int x = 0; x < 7; ++x) {
            WORD color = 0;
            switch(board(x, y)) {
                case EMPTY_SQUARE:  break;
                case PLAYER1:       color = FOREGROUND_BLUE | FOREGROUND_INTENSITY; break;
                case PLAYER2:       color = FOREGROUND_RED | FOREGROUND_INTENSITY; break;
                case PLAYER3:       color = FOREGROUND_GREEN | FOREGROUND_INTENSITY; break;
                case PLAYER4:       color = FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_INTENSITY; break;
                default: /* can't happen */ color = BACKGROUND_RED | BACKGROUND_INTENSITY; break;
            }
            cout << flush;
            SetConsoleTextAttribute(hConsole, color);
            cout << "*" << flush;
            SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
            cout << "|";
        }
        cout << endl << " ---------------" << endl;
    }
    cout << endl;
}

// Return false to quit the game, return true to continue
// 'move' is undefined if user quits
bool askForHumansMove(const Board& board, Move &move)
{
    string move_str;
    while(true) {
        cout << "Your move> ";
        cin >> move_str;
        transform(move_str.begin(), move_str.end(), move_str.begin(), tolower);
        if(move_str == "exit" || move_str == "quit") {
            return false;
        }
        if(!regex_match(move_str, regex("[a-g][1-7][a-g][1-7]"))) {
            cout << "Move format is (e.g.) a1b2" << endl;
            continue;
        }
        move.src.x = move_str[0] - 'a';
        move.src.y = move_str[1] - '1';
        move.dst.x = move_str[2] - 'a';
        move.dst.y = move_str[3] - '1';
        if(checkLegalMove(board, PLAYER1, move)) {
            return true;
        }
        cout << "That is not a legal move." << endl;
    }
}

void decideCpusMove(const Board& board, Move& move)
{
    using namespace std::chrono;
    /*vector<Move> moves;
    findAllPossibleMoves(board, PLAYER2, moves);
    assert(moves.size() > 0);
    move = moves[0];*/
    initZobristTable();
    int nodes_searched = 0;
    steady_clock::time_point t1 = steady_clock::now();
    negamax(board, -INFINITY, INFINITY, move, nodes_searched);
    //mtdf(board, move, nodes_searched);
    steady_clock::time_point t2 = steady_clock::now();
    duration<double> secs = duration_cast<duration<double>>(t2 - t1);
    cout << "Searched " << nodes_searched << " nodes in " << secs.count() << " seconds (" << int(nodes_searched / secs.count()) << " nodes/sec)" << endl;
}
