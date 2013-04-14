#ifndef SPLOT_NEGAMAX_HPP
#define SPLOT_NEGAMAX_HPP

#include "Board.hpp"
#include "moves.hpp"

const bool ENABLE_ALPHA_BETA = true;
const bool ENABLE_ZOBRIST = true;
const int MAX_PLY = 7;      // must be > 0

// Arbitrary value. Should fit into a short (for zobrist hashing).
// It's tempting to use SHORT_MIN and SHORT_MAX for -inf and +inf, but this is
// wrong, because -SHORT_MIN is still SHORT_MIN! (Thanks, two's complement.)
const int INFINITY = 10000;

int negamax(const Board& board, int alpha, int beta, Move& move, int& nodes_searched);
int mtdf(const Board& board, Move& move, int& nodes_searched);

#endif
