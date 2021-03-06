#include <algorithm>
#include <cassert>
#include <cctype>
#include <chrono>
#include <fstream>
#include <iostream>
#include <random>
#include <regex>
#include <string>
#include <sstream>
#include <vector>

// Need Win32 to print pretty colors
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "Board.hpp"
#include "moves.hpp"
#include "ai.hpp"
#include "zobrist.hpp"

using namespace std;

mt19937 g_rng;

void initBoard(Board& board);
void drawBoard(const Board& board);
bool askForWhichAI(int &which_ai);
bool askForHumansMove(Board& board, Move& move);
void decideCpusMove(const Board& board, Move& move, int which_ai);
void saveGame(const Board& board, const std::string& filename);
void loadGame(Board& board, const std::string& filename);

enum WhichAI {
    AI_RANDOM_MOVE,
    AI_NEGAMAX,
    AI_NEGAMAX_ITERATIVE,
    AI_MTDF,
    NUM_AIS
};

int main()
{
    cout << "Built on " << __DATE__ << endl << endl;

    // Seed RNG
    std::random_device rd;
    g_rng.seed(rd());

    // Print big numbers with thousands separators
    locale loc("");
    cout.imbue(loc);

    int which_ai;
    if(!askForWhichAI(which_ai)) {
        return 0;
    }

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
            decideCpusMove(board, move, which_ai);
            /*** DEBUG ***/
            //decideCpusMove(board, move, AI_NEGAMAX_WITHOUT_BB);
            //decideCpusMove(board, move, AI_NEGAMAX_WITH_BB);
            /*** END DEBUG ***/
            makeMove(board, move);
            drawBoard(board);
        }
    }

    return 0;
}

void initBoard(Board& board)
{
    for(int y = 0; y < BOARD_SIZE; ++y) {
        for(int x = 0; x < BOARD_SIZE; ++x) {
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
    for(int y = 0; y < BOARD_SIZE; ++y) {
        cout << (y + 1) << "|";
        for(int x = 0; x < BOARD_SIZE; ++x) {
            WORD color = 0;
            switch(board(x, y)) {
              case EMPTY_SQUARE:    break;
              case PLAYER1:         color = FOREGROUND_BLUE | FOREGROUND_INTENSITY; break;
              case PLAYER2:         color = FOREGROUND_RED | FOREGROUND_INTENSITY; break;
              case PLAYER3:         color = FOREGROUND_GREEN | FOREGROUND_INTENSITY; break;
              case PLAYER4:         color = FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_INTENSITY; break;
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

// Return value is false if user chooses "exit", true otherwise
bool askForWhichAI(int& which_ai)
{
    string which_ai_str;
    while(true) {
        cout << "CHOOSE YOUR OPPONENT" << endl;
        cout << "====================" << endl;
        cout << "1) Complete moron" << endl;
        cout << "2) Negamax" << endl;
        cout << "3) Negamax with iterative deepening" << endl;
        cout << "4) MTD(f)" << endl;
        cout << endl;
        cout << "Enter a number> ";
        cin >> which_ai_str;
        transform(which_ai_str.begin(), which_ai_str.end(), which_ai_str.begin(), tolower);
        if(which_ai_str == "exit" || which_ai_str == "quit") {
            return false;
        }
        stringstream convert(which_ai_str);
        if(!(convert >> which_ai)) {
            // Set which_ai to something that will fail
            which_ai = 0;
        }
        // Make which_ai zero-based
        --which_ai;
        if(which_ai >= 0 && which_ai < NUM_AIS) {
            return true;
        }
        cout << endl << "Beg your pardon?" << endl << endl;
    }
}

// Return false to quit the game, return true to continue
// 'move' is undefined if user quits
// board is non-const because of "load" command
bool askForHumansMove(Board& board, Move &move)
{
    string move_str;
    while(true) {
        cout << "Your move> ";
        cin >> move_str;
        transform(move_str.begin(), move_str.end(), move_str.begin(), tolower);
        if(move_str == "exit" || move_str == "quit") {
            return false;
        }
        if(move_str == "save") {
            saveGame(board, "save.sav");
            continue;
        }
        if(move_str == "load") {
            loadGame(board, "save.sav");
            drawBoard(board);
            continue;
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

void decideCpusMove(const Board& board, Move& move, int which_ai)
{
    using namespace std::chrono;
    if(CLEAR_ZOBRIST_EVERY_TIME) {
        initZobristTable();
    }

    std::vector<int> square_order(NUM_SQUARES);
    std::vector<int> jump_order(NUM_JUMPS);
    for(int i = 0; i < NUM_SQUARES; ++i) {
        square_order.at(i) = i;
    }
    for(int i = 0; i < NUM_JUMPS; ++i) {
        jump_order.at(i) = i;
    }
    if(ENABLE_RANDOMNESS) {
        std::shuffle(square_order.begin(), square_order.end(), g_rng);
    }

    int nodes_searched = 0;
    AiFuncPtr ai;
    switch(which_ai) {
      case AI_RANDOM_MOVE:                  ai = random_move; break;
      case AI_NEGAMAX:                      ai = negamax_bb; break;
      case AI_NEGAMAX_ITERATIVE:            ai = negamax_iterative_bb; break;
      case AI_MTDF:                         ai = mtdf_bb; break;
      default:                              assert(false);
    }
    steady_clock::time_point t1 = steady_clock::now();
    int score = ai(board, square_order, jump_order, move, nodes_searched);
    steady_clock::time_point t2 = steady_clock::now();
    duration<double> secs = duration_cast<duration<double>>(t2 - t1);
    cout << "Searched " << nodes_searched << " nodes in " << secs.count() << " seconds";
    if(secs.count() > 0) {
        cout << " (" << int(nodes_searched / secs.count()) << " nodes/sec)";
    }
    cout << endl << "CPU's estimated score for this move: " << score << endl;
}

// @TODO@ -- error checking; exception safety?
void saveGame(const Board& board, const std::string& filename)
{
    std::ofstream outfile(filename);
    if(outfile.fail()) {
        std::cout << "Failed to write file." << std::endl;
    }
    for(int y = 0; y < BOARD_SIZE; ++y) {
        for(int x = 0; x < BOARD_SIZE; ++x) {
            outfile << unsigned char(board(x, y));
        }
    }
    std::cout << "Game saved." << std::endl;
    outfile.close();
}

// @TODO@ -- error checking; exception safety?
void loadGame(Board& board, const std::string& filename)
{
    std::ifstream infile(filename);
    if(infile.fail()) {
        std::cout << "Failed to load file." << std::endl;
    }
    for(int y = 0; y < BOARD_SIZE; ++y) {
        for(int x = 0; x < BOARD_SIZE; ++x) {
            unsigned char player;
            infile >> player;
            board(x, y) = Player(player);
        }
    }
    infile.close();
}
