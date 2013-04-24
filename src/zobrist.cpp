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
ZobristValue& getZobristValue(const Board& board, int player_sign)
{
    ZobristHash hash = calcHash(board, player_sign);
    return getZobristValueImpl(hash);
}

void setZobristValue(const Board& board, int player_sign, ZobristValue& value)
{
    ZobristHash hash = calcHash(board, player_sign);
    setZobristValueImpl(hash, value);
}


ZobristValue& getZobristValueBB(Bitboard player1, Bitboard player2, int player_sign)
{
    ZobristHash hash = calcHashBB(player1, player2, player_sign);
    return getZobristValueImpl(hash);
}

void setZobristValueBB(Bitboard player1, Bitboard player2, int player_sign, ZobristValue& value)
{
    ZobristHash hash = calcHashBB(player1, player2, player_sign);
    setZobristValueImpl(hash, value);
}

namespace
{

ZobristValue& getZobristValueImpl(ZobristHash hash)
{
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

void setZobristValueImpl(ZobristHash hash, ZobristValue& value)
{
    value.full_hash = hash;
    zobrist_table[hash % ZOBRIST_TABLE_SIZE] = value;
}

ZobristHash calcHash(const Board& board, int player_sign)
{
    ZobristHash hash = 0;
    size_t count = 0;
    for(int y = 0; y < BOARD_SIZE; ++y) {
        for(int x = 0; x < BOARD_SIZE; ++x) {
            Player board_cell = board(x, y);
            assert(board_cell <= PLAYER2);
            if(board_cell != EMPTY_SQUARE) {
                hash ^= ZOBRIST_CODES[board_cell - 1][count];
            }
            ++count;
        }
    }
    if(player_sign == 1) {
        hash ^= PLAYER2_TURN_CODE;
    }
    return hash;
}

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
    assert(hash == old_calcHashBB(player1, player2, player_sign));
    return hash;
}

ZobristHash old_calcHashBB(Bitboard player1, Bitboard player2, int player_sign)
{
    ZobristHash hash = 0;
    Bitboard bit = 1;
    for(int square_num = 0; square_num < NUM_SQUARES; ++square_num) {
        if(player1 & bit) {
            hash ^= ZOBRIST_CODES[0][square_num];
        } else if(player2 & bit) {
            hash ^= ZOBRIST_CODES[1][square_num];
        }
        bit <<= 1;
    }
    if(player_sign == 1) {
        hash ^= PLAYER2_TURN_CODE;
    }
    return hash;
}

}
