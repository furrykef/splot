#ifndef SPLOT_BOARD_HPP
#define SPLOT_BOARD_HPP

#include <cassert>
#include <cstring>

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
        return m_boarddata[coord.x][coord.y];
    }

    Player &operator()(int x, int y)
    {
        return (*this)(Coord(x, y));
    }

    // @TODO@ -- fix duplicate code
    const Player &operator()(const Coord& coord) const
    {
        assert(isInRange(coord));
        return m_boarddata[coord.x][coord.y];
    }

    // @TODO@ -- fix duplicate code
    const Player &operator()(int x, int y) const
    {
        return (*this)(Coord(x, y));
    }

    static bool isInRange(const Coord& coord)
    {
        return (coord.x >= 0 && coord.x < 7 &&
                coord.y >= 0 && coord.y < 7);
    }

    static bool isOutOfRange(const Coord& coord)
    {
        return !isInRange(coord);
    }

    int countPieces(Player player) const
    {
        int count = 0;
        for(int y = 0; y < 7; ++y) {
            for(int x = 0; x < 7; ++x) {
                if(m_boarddata[x][y] == player) {
                    ++count;
                }
            }
        }
        return count;
    }

  private:
    Player m_boarddata[7][7];
};

#endif
