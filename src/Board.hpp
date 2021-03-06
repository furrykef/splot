#ifndef SPLOT_BOARD_HPP
#define SPLOT_BOARD_HPP

#include <cassert>
#include <cstring>

const int BOARD_SIZE = 7;
const int NUM_SQUARES = BOARD_SIZE*BOARD_SIZE;

// Order matters! calcHash assumes EMPTY_SQUARE = 0, etc.
enum Player
{
    PLAYER_NONE = 0,
    EMPTY_SQUARE = 0,
    PLAYER1,
    PLAYER2,
    PLAYER3,
    PLAYER4
};

struct Coord
{
    int x, y;

    Coord(int x_=0, int y_=0)
    {
        x = x_;
        y = y_;
    }

    bool operator==(const Coord& rhs) const
    {
        return x == rhs.x && y == rhs.y;
    }

    bool operator!=(const Coord& rhs) const
    {
        return !(*this == rhs);
    }
};

class Board
{
  public:
    Board() {}

    Board(const Board& other)
    {
        memcpy(m_boarddata, other.m_boarddata, sizeof(m_boarddata));
    }

    Player &operator()(const Coord& coord)
    {
        assert(isInRange(coord));
        return m_boarddata[coord.y][coord.x];
    }

    Player &operator()(int x, int y)
    {
        return (*this)(Coord(x, y));
    }

    // @TODO@ -- fix duplicate code
    const Player &operator()(const Coord& coord) const
    {
        assert(isInRange(coord));
        return m_boarddata[coord.y][coord.x];
    }

    // @TODO@ -- fix duplicate code
    const Player &operator()(int x, int y) const
    {
        return (*this)(Coord(x, y));
    }

    static bool isInRange(const Coord& coord)
    {
        return (coord.x >= 0 && coord.x < BOARD_SIZE &&
                coord.y >= 0 && coord.y < BOARD_SIZE);
    }

    static bool isOutOfRange(const Coord& coord)
    {
        return !isInRange(coord);
    }

    int countPieces(Player player) const
    {
        int count = 0;
        for(int y = 0; y < BOARD_SIZE; ++y) {
            for(int x = 0; x < BOARD_SIZE; ++x) {
                if(m_boarddata[y][x] == player) {
                    ++count;
                }
            }
        }
        return count;
    }

  private:
    Player m_boarddata[BOARD_SIZE][BOARD_SIZE];
};

#endif
