#include "bitboards.hpp"

// Technique stolen from: http://stackoverflow.com/questions/2709430/count-number-of-bits-in-a-64-bit-long-big-integer
int countSetBits(Bitboard bitboard)
{
    bitboard = bitboard - ((bitboard >> 1) & 0x5555555555555555LL);
    bitboard = (bitboard & 0x3333333333333333LL) + ((bitboard >> 2) & 0x3333333333333333LL);
    return (((bitboard + (bitboard >> 4)) & 0xF0F0F0F0F0F0F0FLL) * 0x101010101010101LL) >> 56;
}
