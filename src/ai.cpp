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

int mtdf_impl(const Board& board, int depth, int f, const std::vector<int>& square_order, const std::vector<int>& jump_order, bool& horizon_found, Move& move_out, int& nodes_searched, NegamaxRootFuncPtr fp_negamax_root)
{
    horizon_found = false;
    int score = f;
    int lower_bound = -INFINITY;
    int upper_bound = INFINITY;
    int beta;
    do {
        Move move;
        bool horizon_found_this_iteration;
        beta = (score == lower_bound) ? score + 1 : score;
        score = fp_negamax_root(board, depth, beta - 1, beta, square_order, jump_order, horizon_found_this_iteration, move, nodes_searched);
        horizon_found = horizon_found || horizon_found_this_iteration;
        if(score < beta) {
            // Failed low
            upper_bound = score;
        } else {
            // Failed high
            lower_bound = score;
            move_out = move;
        }
    } while(lower_bound < upper_bound);

    return score;
}
