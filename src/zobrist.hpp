#ifndef SPLOT_ZOBRIST_HPP
#define SPLOT_ZOBRIST_HPP

#include <cstdint>
#include <unordered_map>
#include "Board.hpp"
#include "moves.hpp"
#include "bitboards.hpp"

typedef std::uint64_t ZobristHash;

// Must be unsigned to work correctly as a 2-bit bitfield!
enum ScoreType : unsigned
{
    SCORE_EXACT,
    SCORE_ALPHA,
    SCORE_BETA
};

#pragma pack(1)
struct ZobristValue
{
    ZobristValue() {}

    ZobristValue(short score_, ScoreType score_type_, int depth_, BitboardMove best_move_=BitboardMove(BBMOVE_NONE, 0, 0))
        : score(score_),
          score_type(score_type_),
          depth(depth_),
          best_move(best_move_)
    {
    }

    ZobristHash full_hash;
    short score;
    ScoreType score_type : 2;
    signed depth : 6;                   // Use -1 to signify invalid
    BitboardMove best_move;
};
#pragma pack()

void initZobristTable();
ZobristValue getZobristValue(const Board& board, int player_sign);
void setZobristValue(const Board& board, int player_sign, ZobristValue &value);
ZobristValue getZobristValueBB(Bitboard player1, Bitboard player2, int player_sign);
void setZobristValueBB(Bitboard player1, Bitboard player2, int player_sign, ZobristValue &value);

#endif
