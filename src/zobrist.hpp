#ifndef SPLOT_ZOBRIST_HPP
#define SPLOT_ZOBRIST_HPP

#include <cstdint>
#include <unordered_map>
#include "Board.hpp"

typedef std::uint64_t ZobristHash;

// Must be unsigned to work correctly as a 2-bit bitfield!
enum ScoreType : unsigned
{
    SCORE_EXACT,
    SCORE_ALPHA,
    SCORE_BETA
};

enum BestMoveType : unsigned
{
    BESTMOVE_NONE,
    BESTMOVE_CLONE,
    BESTMOVE_JUMP
};

#pragma pack(1)
struct ZobristValue
{
    ZobristValue()
    {
    }

    ZobristValue(short score_, ScoreType score_type_, int depth_)
    {
        score = score_;
        score_type = score_type_;
        depth = depth_;
    }

    ZobristHash full_hash;
    short score;
    ScoreType score_type : 2;
    signed depth : 6;                   // Use -1 to signify invalid
    BestMoveType best_move_type : 2;
    unsigned square : 6;                // Destination if clone move, source if jump move
    unsigned char jump_type;            // 16 possible values
};
#pragma pack()

void initZobristTable();
ZobristValue getZobristValue(const Board& board, int player_sign);
void setZobristValue(const Board& board, int player_sign, ZobristValue &value);

#endif
