# Written for Python 2.7
from __future__ import division
import sys

# True random numbers generated with HotBits: http://www.fourmilab.ch/hotbits/
ZOBRIST_CODES = [
    [0xA4B992578B5B3456, 0xF330A30C9D0730D9, 0xB3E85D8D02B651F1, 0x573510FFF1D1F459, 0xED1AEE5209AF033D,
     0xFA38FBC2CB4792E9, 0x36EFBF736EEF226B, 0x11FF729BC72587A6, 0xF76844CEE5CFFD46, 0x81B69742FDF65311,
     0xF9B3F146F21B28FA, 0x7B21F2EB7BDAB97E, 0xBCA3C499F196C1EB, 0x964031EBA47FBB2B, 0xF023A91ED963BA6E,
     0x8BA8183A38D4B9D9, 0xFC03E2F903B3E48F, 0xB558C2B52F644F25, 0xF516A6AB6FCE4BB1, 0x33611E32EE3FC9D8,
     0x9507F661546B9800, 0xDE593C442E032002, 0x83C2BD47E38ECEBF, 0xC8922212ECB30D57, 0xF813503AFE497776,
     0x2B78DF97032DA10E, 0x2698FD5B97495A66, 0xB0CF0E39CC9A879A, 0x945C16AE586C10A2, 0xD5722B36A59F17FB,
     0xCAEE5BD9374402FD, 0x2FAFC38E53C62926, 0xF0CA14E075B6FA38, 0x3F156942D16FB555, 0xB7A1962E378C4D9A,
     0x1EF8EFCECC037F02, 0x1C9B68D26CD8FC4D, 0xDF06DE55036678C6, 0xBD0A763B7430BBAB, 0xE415426270218010,
     0xFE1AB2F22F514F39, 0x36956AC566C725BD, 0x34613723B5FCAB2F, 0x1B42202AB7A6744E, 0x24EAC324AD759F43,
     0xC309A33D13C5955A, 0xFB1D0417BF02CCC0, 0x848ABD77A6C220D5, 0x4812B8944701C8E8],
    [0xD9AE6A4E6583BD30, 0x70E216DD31C98AEC, 0x339C23151BA2B545, 0xDC0A792B26A8721C, 0xA1850E0102E9BC19,
     0x9EA4F6559019C42F, 0x30DC375B8402DE77, 0x24C417C393D2D28F, 0xB37BD74950098B48, 0x444D37D19D9F4CE8,
     0x28DBCBF4DC921453, 0x4139983678F19AC4, 0x08196401FDDE3DD5, 0x4C6D89FDD450D209, 0xD15BF0A129B6C39F,
     0xEE0E9FBB6AE450FB, 0xB747B112812BE75F, 0xCF4248576AFC6B91, 0xBD588171D1A0FA86, 0xC3018ABD5D4BA7E7,
     0x2BC2E72AF052C597, 0xDAD519E74E02FC61, 0x48EF9808BF97BAF9, 0xD818E726BB5800F6, 0x3ACDB8D8C295B948,
     0xE558506F1ECC9FA8, 0x8C54B57960C398B0, 0x0B47447208A63BC8, 0xF3E190A6E309C035, 0x6A338B19C28D810C,
     0x975037209DE7C464, 0x44FFC7127AEB0CF2, 0xCAD98E38BF6E2439, 0xE21839E7F474A876, 0x8E133351F3A03746,
     0x48A9006BFDD06CF1, 0x2326D156C5158C6D, 0xA439594E6AC0FBBF, 0x88DBB962A6A90167, 0x96538736374BD5C3,
     0x90A43EBDEE961048, 0x70D451C2F0CFE557, 0x19064D64D8D4BEF8, 0x1CE9063FBADD8A20, 0x5D876EF94A5B67A1,
     0x4699437680BAF525, 0xBA4F2292119B0FE1, 0xD1F06E24324D4786, 0x1DB29BC2DB4959FE]
]

def main():
    ZOBRIST_CODES_OUT = write_zobrist_codes()
    ZOBRIST_CODES_BB = gen_zobrist_bb()
    BITBOARD_SURROUNDS = gen_surrounds()
    JUMP_COORDS, BITBOARD_JUMPS = gen_jumps()

    print ("""
// Generated with gen_lookup_tables.py
// DO NOT MODIFY THIS FILE. Modify gen_lookup_tables.py and generate a new one.
#include "bitboards.hpp"
#include "zobrist.hpp"

const ZobristHash ZOBRIST_CODES[2][NUM_SQUARES] = {
    %(ZOBRIST_CODES_OUT)s
};

const ZobristHash ZOBRIST_CODES_BB[2][4][0x10000] = {
    %(ZOBRIST_CODES_BB)s
};

const Bitboard BITBOARD_SURROUNDS[NUM_SQUARES] = {
    %(BITBOARD_SURROUNDS)s
};

const BitboardJump BITBOARD_JUMPS[NUM_SQUARES][NUM_JUMPS] = {
    %(BITBOARD_JUMPS)s
};

const Coord JUMP_COORDS[NUM_JUMPS] = {
    %(JUMP_COORDS)s
};
""") % locals()


def write_zobrist_codes():
    out = []
    for num_player in xrange(2):
        row = ["0x{0:016x}LL".format(i) for i in ZOBRIST_CODES[num_player]]
        out.append("{%s}" % ",\n     ".join(row))
    return ",\n    ".join(out)


def gen_zobrist_bb():
    out = []
    for player_num in xrange(2):
        out.append("{%s}" % gen_zobrist_bb_word_nums(player_num))
    return ",\n    ".join(out)

def gen_zobrist_bb_word_nums(player_num):
    out = []
    for word_num in xrange(4):
        out.append("{%s}" % gen_zobrist_bb_words(player_num, word_num))
    return ",\n     ".join(out)

def gen_zobrist_bb_words(player_num, word_num):
    out = []
    for word in xrange(0x10000):
        hash_fragment = calc_hash_fragment(player_num, word_num, word)
        out.append("0x{0:016x}LL".format(hash_fragment))
    return ",\n      ".join(out)

def calc_hash_fragment(player_num, word_num, bitboard_fragment):
    bit = 1
    hash = 0
    for square_num in xrange(16*word_num, min(16*(word_num + 1), 7*7)):
        if bitboard_fragment & bit:
            hash ^= ZOBRIST_CODES[player_num][square_num]
        bit <<= 1
    return hash


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
