#include <cstdlib>
#include <vector>

#include "Board.hpp"
#include "moves.hpp"

using namespace std;

bool hasLegalMove(const Board& board, Player player)
{
    vector<Move> moves;
    findAllPossibleMoves(board, player, moves);
    return moves.size() > 0;
}

void findAllPossibleMoves(const Board& board, Player player, vector<Move>& moves)
{
    for(int y = 0; y < 7; ++y) {
        for(int x = 0; x < 7; ++x) {
            if(board(x, y) == player) {
                appendMoves(board, x, y, moves);
            }
        }
    }
}

bool appendMoves(const Board& board, int src_x, int src_y, vector<Move>& moves)
{
    assert(board.isInRange(Coord(src_x, src_y)));
    for(int dst_y = src_y - 2; dst_y <= src_y + 2; ++dst_y) {
        for(int dst_x = src_x - 2; dst_x <= src_x + 2; ++dst_x) {
            if(board.isInRange(Coord(dst_x, dst_y)) && board(dst_x, dst_y) == EMPTY_SQUARE) {
                Move move = {Coord(src_x, src_y), Coord(dst_x, dst_y)};
                moves.push_back(move);
            }
        }
    }
    return false;
}

bool checkLegalMove(const Board& board, Player player, const Move& move)
{
    if(board.isOutOfRange(move.src) ||
       board.isOutOfRange(move.dst) ||
       board(move.src.x, move.src.y) != player ||
       board(move.dst.x, move.dst.y) != EMPTY_SQUARE) {
        return false;
    }
    int distance_x = abs(move.dst.x - move.src.x);
    int distance_y = abs(move.dst.y - move.src.y);
    return distance_x <= 2 && distance_y <= 2;
}

void makeMove(Board& board, const Move& move)
{
    assert(board.isInRange(move.src));
    assert(board.isInRange(move.dst));
    assert(move.src != move.dst);

    board(move.dst.x, move.dst.y) = board(move.src.x, move.src.y);

    // Captures
    for(int y = move.dst.y - 1; y <= move.dst.y + 1; ++y) {
        for(int x = move.dst.x - 1; x <= move.dst.x + 1; ++x) {
            if(board.isInRange(Coord(x, y)) && board(x, y) != EMPTY_SQUARE) {
                board(x, y) = board(move.src.x, move.src.y);
            }
        }
    }

    // Check if piece has jumped
    int distance_x = abs(move.dst.x - move.src.x);
    int distance_y = abs(move.dst.y - move.src.y);
    assert(distance_x <= 2);
    assert(distance_y <= 2);
    if(distance_x == 2 || distance_y == 2) {
        // Piece has jumped; remove it from source square
        board(move.src.x, move.src.y) = EMPTY_SQUARE;
    }
}
