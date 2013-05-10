#include <algorithm>
#include <random>
#include <vector>

#include "Board.hpp"
#include "moves.hpp"
#include "ai.hpp"

using namespace std;

extern mt19937 g_rng;

int random_move(const Board& board, const std::vector<int>& square_order, const std::vector<int>& jump_order, Move& move_out, int& nodes_searched)
{
    vector<Move> moves;
    // @TODO@ -- assumes AI is PLAYER2
    findAllPossibleMoves(board, PLAYER2, moves);
    assert(moves.size() > 0);
    move_out = moves[g_rng() % moves.size()];
    return 0;
}

int mtdf_impl(const Board& board, int depth, int f, const std::vector<int>& square_order, const std::vector<int>& jump_order, Move& move_out, int& nodes_searched, NegamaxRootFuncPtr fp_negamax_root)
{
    int score = f;
    int lower_bound = -INFINITY;
    int upper_bound = INFINITY;
    int beta;
    do {
        Move move;
        beta = (score == lower_bound) ? (score + 1) : score;
        score = fp_negamax_root(board, depth, beta - 1, beta, square_order, jump_order, move, nodes_searched);
        if(score < beta) {
            // Failed low
            upper_bound = score;
        } else {
            // Failed high
            lower_bound = score;
            assert(checkLegalMove(board, PLAYER2, move));   // @TODO@ -- assumes AI is player 2
            move_out = move;
        }
    } while(lower_bound < upper_bound);

    return score;
}
