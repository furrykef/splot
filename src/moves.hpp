#ifndef SPLOT_MOVES_HPP
#define SPLOT_MOVES_HPP

#include <vector>
#include "Board.hpp"

struct Move
{
    Coord src, dst;
};

enum BitboardMoveType : unsigned
{
    BBMOVE_NONE,    // Not a null move! This is more like "not a move" or "invalid"; it's like a null pointer for moves
    BBMOVE_CLONE,
    BBMOVE_JUMP,
    BBMOVE_NULL
};

// Used in transposition table
#pragma pack(1)
struct BitboardMove
{
    BitboardMove() {}

    BitboardMove(BitboardMoveType move_type_, unsigned int square_, unsigned int jump_type_)
        : move_type(move_type_),
          square(square_),
          jump_type(jump_type_)
    {
    }

    BitboardMoveType move_type : 2;
    unsigned square : 6;                // Destination if clone move, source if jump move
    unsigned char jump_type;            // 16 possible values
};
#pragma pack()

bool hasLegalMove(const Board& board, Player player);
void findAllPossibleMoves(const Board& board, Player player, std::vector<Move>& moves);
bool appendMoves(const Board& board, int src_x, int src_y, std::vector<Move>& moves);
bool checkLegalMove(const Board& board, Player player, const Move& move);
void makeMove(Board& board, const Move& move);

#endif
