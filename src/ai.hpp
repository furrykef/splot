#ifndef SPLOT_NEGAMAX_HPP
#define SPLOT_NEGAMAX_HPP

#include "Board.hpp"
#include "moves.hpp"

const bool ENABLE_ALPHA_BETA = true;
const bool ENABLE_ZOBRIST = true;
const int MAX_PLY = 8;                  // must be > 0
const bool ENABLE_BEST_FIRST = true;
const bool ENABLE_NULL_MOVE = false;
const int NULL_MOVE_REDUCTION = 2;      // must be > 0

// Arbitrary value. Should fit into a short (for zobrist hashing).
// It's tempting to use SHRT_MIN and SHRT_MAX for -inf and +inf, but this is
// wrong, because -SHRT_MIN is still SHRT_MIN! (Thanks, two's complement.)
const int INFINITY = 10000;

typedef int (*AiFuncPtr)(const Board& board, Move& move, int& nodes_searched);
typedef int (*NegamaxRootFuncPtr)(const Board& board, int depth, int alpha, int beta, Move& move_out, int& nodes_searched);

int random_move(const Board& board, Move& move, int& nodes_searched);
int negamax(const Board& board, Move& move, int& nodes_searched);
int negamax_iterative(const Board& board, Move& move, int& nodes_searched);
int mtdf(const Board& board, Move& move, int& nodes_searched);
int mtdf_impl(const Board &board, int depth, int f, Move& move_out, int& nodes_searched, NegamaxRootFuncPtr fp_negamax_root);
int negamax_bb(const Board& board, Move& move, int& nodes_searched);
int negamax_iterative_bb(const Board& board, Move& move, int& nodes_searched);
int mtdf_bb(const Board& board, Move& move, int& nodes_searched);

#endif
