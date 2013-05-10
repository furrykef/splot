#include <cassert>
#include <cstdlib>
#include <functional>
#include "zobrist.hpp"

const size_t ZOBRIST_TABLE_SIZE = 64 * 0x100000;

ZobristValue zobrist_table[ZOBRIST_TABLE_SIZE];

// XORed into hash when it's player 2's turn (@TODO@ -- codes for player 3, player 4)
const ZobristHash PLAYER2_TURN_CODE = 0x431D89EC63B226D7LL;

namespace
{

ZobristValue& getZobristValueImpl(ZobristHash hash);
void setZobristValueImpl(ZobristHash hash, ZobristValue& value);
ZobristHash calcHash(const Board& board, int player_sign);
ZobristHash calcHashBB(Bitboard player1, Bitboard player2, int player_sign);
ZobristHash old_calcHashBB(Bitboard player1, Bitboard player2, int player_sign);

}

void initZobristTable()
{
    // Mark everything in the table as invalid
    for(ZobristValue& value : zobrist_table) {
        value = ZobristValue();
    }
}

// player_sign is 1 for AI, -1 for human
// Remember, depth of -1 signifies the whole ZobristValue is invalid
ZobristValue& getZobristValueBB(Bitboard player1, Bitboard player2, int player_sign)
{
    ZobristHash hash = calcHashBB(player1, player2, player_sign);
    ZobristValue& value = zobrist_table[hash % ZOBRIST_TABLE_SIZE];
    if(value.full_hash != hash) {
        // The short hash (i.e. mod ZOBRIST_TABLE_SIZE) collided, but the full hash did not.
        // That means this result is spurious; invalidate it.
        // @TODO@ -- is it really wise to clear entry in the actual table?
        // Sometimes the AI does read the transposition table without writing it.
        value = ZobristValue();
        value.full_hash = hash;
    }
    return value;
}

namespace
{

ZobristHash calcHashBB(Bitboard player1, Bitboard player2, int player_sign)
{
    ZobristHash hash = 0;
    hash ^= ZOBRIST_CODES_BB[0][0][0xffff & player1];
    hash ^= ZOBRIST_CODES_BB[0][1][0xffff & (player1 >> 16)];
    hash ^= ZOBRIST_CODES_BB[0][2][0xffff & (player1 >> 32)];
    hash ^= ZOBRIST_CODES_BB[0][3][0xffff & (player1 >> 48)];
    hash ^= ZOBRIST_CODES_BB[1][0][0xffff & player2];
    hash ^= ZOBRIST_CODES_BB[1][1][0xffff & (player2 >> 16)];
    hash ^= ZOBRIST_CODES_BB[1][2][0xffff & (player2 >> 32)];
    hash ^= ZOBRIST_CODES_BB[1][3][0xffff & (player2 >> 48)];
    if(player_sign == 1) {
        hash ^= PLAYER2_TURN_CODE;
    }
    return hash;
}

}
