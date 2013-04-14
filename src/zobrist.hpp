#ifndef SPLOT_ZOBRIST_HPP
#define SPLOT_ZOBRIST_HPP

#include <unordered_map>
#include "Board.hpp"

typedef long long unsigned int ZobristHash;

enum ScoreType
{
    SCORE_EXACT,
    SCORE_ALPHA,
    SCORE_BETA,
};

#pragma pack(1)
struct ZobristValue
{
    ZobristHash full_hash;
    short score;
    ScoreType score_type : 2;
    signed depth : 6;               // Use -1 to signify invalid
};
#pragma pack()

void initZobristTable();
void getZobristValue(const Board& board, int turn, ZobristValue &value);
void setZobristValue(const Board& board, int turn, ZobristValue &value);

#endif
