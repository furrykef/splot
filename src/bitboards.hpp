#ifndef SPLOT_BITBOARDS_HPP
#define SPLOT_BITBOARDS_HPP

#include <cstdint>
#include "moves.hpp"

typedef std::uint64_t Bitboard;

struct BitboardJump
{
    Bitboard dest_square;
    Bitboard capture_radius;
};

const int NUM_JUMPS = 16;
extern const Bitboard BITBOARD_SURROUNDS[NUM_SQUARES];
extern const BitboardJump BITBOARD_JUMPS[NUM_SQUARES][NUM_JUMPS];
extern const Coord JUMP_COORDS[NUM_JUMPS];

int countSetBits(Bitboard bitboard);

// NEVER use ~bitboard! Use this instead to ensure that bits outside the board's bounds are not set to 1.
inline Bitboard invertBitboard(Bitboard bitboard)
{
    return ~bitboard & 0x1ffffffffffffLL;
}

#endif
