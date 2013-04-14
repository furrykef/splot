#ifndef SPLOT_MOVES_HPP
#define SPLOT_MOVES_HPP

#include <vector>
#include "Board.hpp"

struct Move
{
    Coord src, dst;
};

bool hasLegalMove(const Board& board, Player player);
void findAllPossibleMoves(const Board& board, Player player, std::vector<Move>& moves);
bool appendMoves(const Board& board, int src_x, int src_y, std::vector<Move>& moves);
bool checkLegalMove(const Board& board, Player player, const Move& move);
void makeMove(Board& board, const Move& move);

#endif
