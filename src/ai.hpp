#ifndef SPLOT_NEGAMAX_HPP
#define SPLOT_NEGAMAX_HPP

#include <vector>
#include "Board.hpp"
#include "moves.hpp"

const bool ENABLE_RANDOMNESS = true;
const bool ENABLE_ALPHA_BETA = true;
const bool ENABLE_ZOBRIST = true;
const int MAX_PLY = 8;                  // must be > 0
const bool ENABLE_BEST_FIRST = true;
const bool ENABLE_NULL_MOVE = false;
const bool CLEAR_ZOBRIST_EVERY_TIME = true;

// Should be > 0. This number includes the ply of the null move itself,
// so if we used 0, the search depth would not be reduced at all (i.e.,
// we'd flip the player flag without even reducing the depth number).
// That means this value is 1 greater than the R conventionally used.
const int NULL_MOVE_REDUCTION = 2;

// Arbitrary value. Should fit into a short (for zobrist hashing).
// It's tempting to use SHRT_MIN and SHRT_MAX for -inf and +inf, but this is
// wrong, because -SHRT_MIN is still SHRT_MIN! (Thanks, two's complement.)
// Note that a null window of -INFINITY - 1, -INFINITY is permissible.
const int INFINITY = 10000;
const int WIN = INFINITY;
const int LOSS = -WIN;

typedef int (*AiFuncPtr)(const Board& board, const std::vector<int>& square_order, Move& move_out, int& nodes_searched);
typedef int (*NegamaxRootFuncPtr)(const Board& board, int depth, int alpha, int beta, const std::vector<int>& square_order, Move& move_out, int& nodes_searched);

int random_move(const Board& board, const std::vector<int>& square_order, Move& move, int& nodes_searched);
int negamax(const Board& board, const std::vector<int>& square_order, Move& move, int& nodes_searched);
int negamax_iterative(const Board& board, const std::vector<int>& square_order, Move& move, int& nodes_searched);
int mtdf(const Board& board, const std::vector<int>& square_order, Move& move, int& nodes_searched);
int mtdf_impl(const Board &board, int depth, int f, const std::vector<int>& square_order, Move& move, int& nodes_searched, NegamaxRootFuncPtr fp_negamax_root);
int negamax_bb(const Board& board, const std::vector<int>& square_order, Move& move, int& nodes_searched);
int negamax_iterative_bb(const Board& board, const std::vector<int>& square_order, Move& move, int& nodes_searched);
int mtdf_bb(const Board& board, const std::vector<int>& square_order, Move& move, int& nodes_searched);

#endif
