#ifndef SPLOT_ZOBRIST_HPP
#define SPLOT_ZOBRIST_HPP

#include <cstdint>
#include <unordered_map>
#include "Board.hpp"
#include "moves.hpp"
#include "bitboards.hpp"
#include "ai.hpp"

typedef std::uint64_t ZobristHash;

#pragma pack(1)
struct ZobristValue
{
    ZobristValue()
        : lower_bound(-INFINITY),
          upper_bound(INFINITY),
          depth(-1),
          best_move(BitboardMove(BBMOVE_NONE, 0, 0))
    {
    }

    ZobristValue(short lower_bound_, short upper_bound_, int depth_, BitboardMove best_move_=BitboardMove(BBMOVE_NONE, 0, 0))
        : lower_bound(lower_bound_),
          upper_bound(upper_bound_),
          depth(depth_),
          best_move(best_move_)
    {
    }

    ZobristHash full_hash;
    short lower_bound;
    short upper_bound;
    signed char depth;                      // Use -1 to signify invalid
    BitboardMove best_move;
};
#pragma pack()

void initZobristTable();
ZobristValue getZobristValueBB(ZobristHash hash);
void setZobristValueBB(ZobristHash hash, const ZobristValue& value);
ZobristHash calcHashBB(Bitboard player1, Bitboard player2, int player_sign);

extern const ZobristHash ZOBRIST_CODES[2][NUM_SQUARES];
extern const ZobristHash ZOBRIST_CODES_BB[2][4][0x10000];

#endif
