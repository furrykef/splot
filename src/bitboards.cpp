#include "bitboards.hpp"

int countSetBits(Bitboard bitboard)
{
    int count = 0;
    for(; bitboard != 0; bitboard >>= 1) {
        count += bitboard & 1;
    }
    return count;
}
