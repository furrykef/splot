#include <cassert>
#include <cstdlib>
#include <unordered_map>
#include "zobrist.hpp"

const size_t ZOBRIST_TABLE_SIZE = 64 * 0x100000;

ZobristValue zobrist_table[ZOBRIST_TABLE_SIZE];

// True random numbers generated with HotBits: http://www.fourmilab.ch/hotbits/
const ZobristHash ZOBRIST_CODES[7*7*2] = {
    0xA4B992578B5B3456LL, 0xF330A30C9D0730D9LL, 0xB3E85D8D02B651F1LL, 0x573510FFF1D1F459LL, 0xED1AEE5209AF033DLL,
    0xFA38FBC2CB4792E9LL, 0x36EFBF736EEF226BLL, 0x11FF729BC72587A6LL, 0xF76844CEE5CFFD46LL, 0x81B69742FDF65311LL,
    0xF9B3F146F21B28FALL, 0x7B21F2EB7BDAB97ELL, 0xBCA3C499F196C1EBLL, 0x964031EBA47FBB2BLL, 0xF023A91ED963BA6ELL,
    0x8BA8183A38D4B9D9LL, 0xFC03E2F903B3E48FLL, 0xB558C2B52F644F25LL, 0xF516A6AB6FCE4BB1LL, 0x33611E32EE3FC9D8LL,
    0x9507F661546B9800LL, 0xDE593C442E032002LL, 0x83C2BD47E38ECEBFLL, 0xC8922212ECB30D57LL, 0xF813503AFE497776LL,
    0x2B78DF97032DA10ELL, 0x2698FD5B97495A66LL, 0xB0CF0E39CC9A879ALL, 0x945C16AE586C10A2LL, 0xD5722B36A59F17FBLL,
    0xCAEE5BD9374402FDLL, 0x2FAFC38E53C62926LL, 0xF0CA14E075B6FA38LL, 0x3F156942D16FB555LL, 0xB7A1962E378C4D9ALL,
    0x1EF8EFCECC037F02LL, 0x1C9B68D26CD8FC4DLL, 0xDF06DE55036678C6LL, 0xBD0A763B7430BBABLL, 0xE415426270218010LL,
    0xFE1AB2F22F514F39LL, 0x36956AC566C725BDLL, 0x34613723B5FCAB2FLL, 0x1B42202AB7A6744ELL, 0x24EAC324AD759F43LL,
    0xC309A33D13C5955ALL, 0xFB1D0417BF02CCC0LL, 0x848ABD77A6C220D5LL, 0x4812B8944701C8E8LL, 0xD9AE6A4E6583BD30LL,
    0x70E216DD31C98AECLL, 0x339C23151BA2B545LL, 0xDC0A792B26A8721CLL, 0xA1850E0102E9BC19LL, 0x9EA4F6559019C42FLL,
    0x30DC375B8402DE77LL, 0x24C417C393D2D28FLL, 0xB37BD74950098B48LL, 0x444D37D19D9F4CE8LL, 0x28DBCBF4DC921453LL,
    0x4139983678F19AC4LL, 0x08196401FDDE3DD5LL, 0x4C6D89FDD450D209LL, 0xD15BF0A129B6C39FLL, 0xEE0E9FBB6AE450FBLL,
    0xB747B112812BE75FLL, 0xCF4248576AFC6B91LL, 0xBD588171D1A0FA86LL, 0xC3018ABD5D4BA7E7LL, 0x2BC2E72AF052C597LL,
    0xDAD519E74E02FC61LL, 0x48EF9808BF97BAF9LL, 0xD818E726BB5800F6LL, 0x3ACDB8D8C295B948LL, 0xE558506F1ECC9FA8LL,
    0x8C54B57960C398B0LL, 0x0B47447208A63BC8LL, 0xF3E190A6E309C035LL, 0x6A338B19C28D810CLL, 0x975037209DE7C464LL,
    0x44FFC7127AEB0CF2LL, 0xCAD98E38BF6E2439LL, 0xE21839E7F474A876LL, 0x8E133351F3A03746LL, 0x48A9006BFDD06CF1LL,
    0x2326D156C5158C6DLL, 0xA439594E6AC0FBBFLL, 0x88DBB962A6A90167LL, 0x96538736374BD5C3LL, 0x90A43EBDEE961048LL,
    0x70D451C2F0CFE557LL, 0x19064D64D8D4BEF8LL, 0x1CE9063FBADD8A20LL, 0x5D876EF94A5B67A1LL, 0x4699437680BAF525LL,
    0xBA4F2292119B0FE1LL, 0xD1F06E24324D4786LL, 0x1DB29BC2DB4959FELL
};

// XORed into hash when it's player 2's turn (@TODO@ -- codes for player 3, player 4)
const ZobristHash PLAYER2_TURN_CODE = 0x431D89EC63B226D7LL;

namespace
{
ZobristHash calcHash(const Board& board, int player_sign);
ZobristHash calcHashBB(Bitboard player1, Bitboard player2, int player_sign);
}

void initZobristTable()
{
    // Mark everything in the table as invalid
    for(ZobristValue& value : zobrist_table) {
        value.depth = -1;
        value.best_move.move_type = BBMOVE_NONE;
    }
}

// player_sign is 1 for AI, -1 for human
// Remember, depth of -1 signifies the whole ZobristValue is invalid
ZobristValue getZobristValue(const Board& board, int player_sign)
{
    ZobristHash hash = calcHash(board, player_sign);
    ZobristValue value = zobrist_table[hash % ZOBRIST_TABLE_SIZE];
    if(value.full_hash != hash) {
        // The short hash (i.e. mod ZOBRIST_TABLE_SIZE) collided, but the full hash did not.
        // That means this result is spurious; invalidate it.
        value.depth = -1;
    }
    return value;
}

void setZobristValue(const Board& board, int player_sign, ZobristValue& value)
{
    ZobristHash hash = calcHash(board, player_sign);
    value.full_hash = hash;
    zobrist_table[hash % ZOBRIST_TABLE_SIZE] = value;
}


// @TODO@ -- code duplication with non-BB versions
ZobristValue getZobristValueBB(Bitboard player1, Bitboard player2, int player_sign)
{
    ZobristHash hash = calcHashBB(player1, player2, player_sign);
    ZobristValue value = zobrist_table[hash % ZOBRIST_TABLE_SIZE];
    if(value.full_hash != hash) {
        // The short hash (i.e. mod ZOBRIST_TABLE_SIZE) collided, but the full hash did not.
        // That means this result is spurious; invalidate it.
        value.depth = -1;
    }
    return value;
}

// @TODO@ -- code duplication with non-BB versions
void setZobristValueBB(Bitboard player1, Bitboard player2, int player_sign, ZobristValue &value)
{
    ZobristHash hash = calcHashBB(player1, player2, player_sign);
    value.full_hash = hash;
    zobrist_table[hash % ZOBRIST_TABLE_SIZE] = value;
}

namespace
{

ZobristHash calcHash(const Board& board, int player_sign)
{
    ZobristHash hash = 0;
    size_t count = 0;
    for(int y = 0; y < 7; ++y) {
        for(int x = 0; x < 7; ++x) {
            Player board_cell = board(x, y);
            assert(board_cell <= PLAYER2);
            if(board_cell != EMPTY_SQUARE) {
                hash ^= ZOBRIST_CODES[(board_cell - 1) * 7*7 + count++];
            }
        }
    }
    if(player_sign == 1) {
        hash ^= PLAYER2_TURN_CODE;
    }
    return hash;
}

ZobristHash calcHashBB(Bitboard player1, Bitboard player2, int player_sign)
{
    // @TODO@ -- can be optimized with LUTs. Try profiling.
    ZobristHash hash = 0;
    Bitboard bit = 1;
    for(int square_num = 0; square_num < 7*7; ++square_num) {
        if(player1 & bit) {
            hash ^= ZOBRIST_CODES[7*7 + square_num];
        } else if(player2 & bit) {
            hash ^= ZOBRIST_CODES[7*7*2 + square_num];
        }
        bit <<= 1;
    }
    if(player_sign == 1) {
        hash ^= PLAYER2_TURN_CODE;
    }
    return hash;
}

}
