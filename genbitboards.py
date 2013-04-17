# Written for Python 2.7
from __future__ import division
import sys

def main():
    BITBOARD_SURROUNDS = gen_surrounds()
    JUMP_COORDS, BITBOARD_JUMPS = gen_jumps()

    print ("""
// Generated with genbitboards.py
// DO NOT MODIFY THIS FILE. Modify genbitboards.py and generate a new one.
#include "bitboards.hpp"

const Bitboard BITBOARD_SURROUNDS[7*7] = {
    %(BITBOARD_SURROUNDS)s
};

const BitboardJump BITBOARD_JUMPS[49][16] = {
    %(BITBOARD_JUMPS)s
};

const Coord JUMP_COORDS[16] = {
    %(JUMP_COORDS)s
};
""") % locals()


def gen_surrounds():
    bitboards = []
    for y in xrange(7):
        for x in xrange(7):
            bitboards.append(calc_surrounds(x, y))
    return ",\n    ".join(bitboard_to_string(x) for x in bitboards)

def calc_surrounds(x, y):
    if not is_in_range(x, y):
        return 0
    bitboard = 0
    for y2 in xrange(y-1, y+2):
        for x2 in xrange(x-1, x+2):
            if (x2, y2) != (x, y):
                bitboard |= coord_to_bit(x2, y2)
    return bitboard

# Each type of jump has a number from 0 to 15.
# This is the order of the jumps (A = 10, etc.):
#
#   01234
#   F...5
#   E.*.6
#   D...7
#   CBA98
#
# To help the main program translate these into coordinates, we also
# generate a lookup table, so 0 maps to {-2, -2}, 1 maps to (-2, 1), etc.
def gen_jumps():
    # This is like drawing a square clockwise from top-left,
    # taking care not to repeat anything
    jump_coords = []
    for x in xrange(-2, 3):
        jump_coords.append((x, -2))

    for y in xrange(-1, 3):
        jump_coords.append((2, y))

    for x in xrange(1, -3, -1):
        jump_coords.append((x, 2))

    for y in xrange(1, -2, -1):
        jump_coords.append((-2, y))

    jumps = []
    for y in xrange(7):
        for x in range(7):
            jumps.append(gen_jumps_at_square(x, y, jump_coords))

    jump_coords = ",\n    ".join("Coord(%d, %d)" % (x, y) for (x, y) in jump_coords)
    jumps = ",\n    ".join(jumps)
    return jump_coords, jumps

def gen_jumps_at_square(src_x, src_y, jump_coords):
    out = []
    for (offset_x, offset_y) in jump_coords:
        dst_x = src_x + offset_x
        dst_y = src_y + offset_y
        bit = coord_to_bit(dst_x, dst_y)
        surrounds = calc_surrounds(dst_x, dst_y) if bit != 0 else 0
        out.append("{%s, %s}" % (bitboard_to_string(bit), bitboard_to_string(surrounds)))
    return "{%s}" % ",\n     ".join(out)

def bitboard_to_string(bitboard):
    return "0x{0:016x}LL".format(bitboard)

def is_in_range(x, y):
    return 0 <= x < 7 and 0 <= y < 7

def coord_to_bitnum(x, y):
    assert is_in_range(x, y)
    return y*7+x

def coord_to_bit(x, y):
    return 1 << coord_to_bitnum(x, y) if is_in_range(x, y) else 0


if __name__ == '__main__':
    sys.exit(main())
