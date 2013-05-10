#ifndef SPLOT_NEGAMAX_HPP
#define SPLOT_NEGAMAX_HPP

#include <vector>
#include "Board.hpp"
#include "moves.hpp"

const bool ENABLE_RANDOMNESS = true;
const bool ENABLE_ALPHA_BETA = true;
const bool ENABLE_ZOBRIST = true;
const int MAX_PLY = 9;                  // must be > 0
const bool ENABLE_BEST_FIRST = true;
const bool ENABLE_NULL_MOVE = false;
const bool ENABLE_FUTILITY = true;
const int FUTILITY_THRESHOLD = 16;      // player's score can increase at most by 16 on a turn w/ current scoring method (player gains 8 pieces, opponent loses 8 pieces)
const bool CLEAR_ZOBRIST_EVERY_TIME = false;

// Largest value that fits in transposition table's "depth" field.
// Used when the exact score of a position is known (i.e. the game is over).
// That way the position need not be re-searched.
const int INFINITE_PLY = 0x7f;

// Should be > 0. This number includes the ply of the null move itself,
// so if we used 0, the search depth would not be reduced at all (i.e.,
// we'd flip the player flag without even reducing the depth number).
// That means this value is 1 greater than the R conventionally used.
const int NULL_MOVE_REDUCTION = 2;

// Arbitrary value. Should fit into a short (for transposition table). It's
// tempting to use SHRT_MIN for -inf, but this is wrong, because -SHRT_MIN is
// still SHRT_MIN! (Thanks, two's complement.)
const int INFINITY = 0x7fff;

// Must be within the bounds of INFINITY and -INFINITY.
const int WIN = 10000;
const int LOSS = -WIN;

typedef int (*AiFuncPtr)(const Board& board, const std::vector<int>& square_order, const std::vector<int>& jump_order, Move& move_out, int& nodes_searched);
typedef int (*NegamaxRootFuncPtr)(const Board& board, int depth, int alpha, int beta, const std::vector<int>& square_order, const std::vector<int>& jump_order, Move& move_out, int& nodes_searched);

int random_move(const Board& board, const std::vector<int>& square_order, const std::vector<int>& jump_order, Move& move, int& nodes_searched);
int mtdf_impl(const Board &board, int depth, int f, const std::vector<int>& square_order, const std::vector<int>& jump_order, Move& move, int& nodes_searched, NegamaxRootFuncPtr fp_negamax_root);
int negamax_bb(const Board& board, const std::vector<int>& square_order, const std::vector<int>& jump_order, Move& move, int& nodes_searched);
int negamax_iterative_bb(const Board& board, const std::vector<int>& square_order, const std::vector<int>& jump_order, Move& move, int& nodes_searched);
int mtdf_bb(const Board& board, const std::vector<int>& square_order, const std::vector<int>& jump_order, Move& move, int& nodes_searched);

#endif
