#include <algorithm>
#include <random>
#include <vector>

#include "Board.hpp"
#include "moves.hpp"
#include "ai.hpp"
#include "zobrist.hpp"

using namespace std;

extern mt19937 g_rng;

namespace
{
int negamax_root(const Board& board, int depth, int alpha, int beta, const std::vector<int>& square_order, Move& move_out, int& nodes_searched);
int negamax_impl(const Board& board, int depth, int alpha, int beta, int player_sign, int& nodes_searched);
int evalPosition(const Board& board);
void findMoves(const Board& board, Player who, vector<Move>& moves);
void findLegalClones(const Board& board, Player who, vector<Move>& moves);
void appendCloneIfFound(const Board& board, int dst_x, int dst_y, Player who, vector<Move>& moves);
void findLegalJumps(const Board& board, Player who, vector<Move>& moves);
void appendLegalJumps(const Board& board, int src_x, int src_y, vector<Move>& moves);
void appendJumpIfLegal(const Board& board, int src_x, int src_y, int dst_x, int dst_y, vector<Move>& moves);
}

int random_move(const Board& board, const std::vector<int>& square_order, Move& move_out, int& nodes_searched)
{
    vector<Move> moves;
    // @TODO@ -- assumes AI is PLAYER2
    findAllPossibleMoves(board, PLAYER2, moves);
    assert(moves.size() > 0);
    move_out = moves[g_rng() % moves.size()];
    return 0;
}

// TRANSPOSITION TABLE MUST BE INITIALIZED BEFORE CALLING
int negamax(const Board& board, const std::vector<int>& square_order, Move& move_out, int& nodes_searched)
{
    return negamax_root(board, MAX_PLY, -INFINITY, INFINITY, square_order, move_out, nodes_searched);
}

// TRANSPOSITION TABLE MUST BE INITIALIZED BEFORE CALLING
int negamax_iterative(const Board& board, const std::vector<int>& square_order, Move& move_out, int& nodes_searched)
{
    int score;
    for(int depth = 1; depth <= MAX_PLY; ++depth) {
        score = negamax_root(board, depth, -INFINITY, INFINITY, square_order, move_out, nodes_searched);
    }
    return score;
}

// TRANSPOSITION TABLE MUST BE INITIALIZED BEFORE CALLING
// @TODO@ -- code duplication with BB version
int mtdf(const Board& board, const std::vector<int>& square_order, Move& move_out, int& nodes_searched)
{
    int f = 0;
    for(int depth = 1; depth <= MAX_PLY; ++depth) {
        f = mtdf_impl(board, depth, f, square_order, move_out, nodes_searched, negamax_root);
    }
    return f;
}

int mtdf_impl(const Board& board, int depth, int f, const std::vector<int>& square_order, Move& move_out, int& nodes_searched, NegamaxRootFuncPtr fp_negamax_root)
{
    int score = f;
    int lower_bound = -INFINITY;
    int upper_bound = INFINITY;
    int beta;
    do {
        Move move;
        beta = (score == lower_bound) ? score + 1 : score;
        score = fp_negamax_root(board, depth, beta - 1, beta, square_order, move, nodes_searched);
        if(score < beta) {
            // Failed low
            upper_bound = score;
        } else {
            // Failed high or is exact score
            lower_bound = score;
            move_out = move;
        }
    } while(lower_bound < upper_bound);

    return score;
}

namespace
{

// @TODO@ -- merge functionality with negamax_impl
int negamax_root(const Board& board, int depth, int alpha, int beta, const std::vector<int>& square_order, Move& move_out, int& nodes_searched)
{
    ++nodes_searched;

    vector<Move> moves;

    // @TODO@ -- Assumes AI is PLAYER2
    findMoves(board, PLAYER2, moves);
    assert(moves.size() > 0);

    // Best score is distinguished from alpha because we need to output a
    // decent move even if we fail low (thanks, MTD(f)).
    int best_score = -INFINITY;
    for(const Move& move : moves) {
        Board board_after_move(board);
        makeMove(board_after_move, move);
        int score = -negamax_impl(board_after_move, depth - 1, -beta, -alpha, -1, nodes_searched);
        if(score > best_score) {
            best_score = score;
            move_out = move;
        }
        if(score >= beta) {
            return score;
        }
        alpha = std::max(alpha, score);
    }

    return alpha;
}

// player_sign is 1 for Max (i.e. this AI player), -1 for Min
int negamax_impl(const Board& board, int depth, int alpha, int beta, int player_sign, int& nodes_searched)
{
    ++nodes_searched;

    if(!ENABLE_ALPHA_BETA) {
        alpha = -INFINITY;
        beta = INFINITY;
    }

    ZobristValue& zv = getZobristValue(board, player_sign);
    if(ENABLE_ZOBRIST) {
        if(zv.depth >= depth) {
            if(zv.lower_bound >= beta) {
                return zv.lower_bound;
            } else if(zv.upper_bound <= alpha) {
                return zv.upper_bound;
            }
            alpha = std::max(alpha, int(zv.lower_bound));
            beta = std::min(beta, int(zv.upper_bound));
        }
        zv.depth = depth;
    }

    if(depth == 0) {
        int score = player_sign * evalPosition(board);
        zv.lower_bound = score;
        zv.upper_bound = score;
        zv.best_move.move_type = BBMOVE_NONE;
        return score;
    }

    vector<Move> moves;
    // @TODO@ -- Assumes AI is PLAYER2
    findMoves(board, (player_sign == 1) ? PLAYER2 : PLAYER1, moves);

    if(moves.size() == 0) {
        // @TODO@ -- This is not correct behavior!
        // Should check if game over and if so return position evaluation.
        // Otherwise, should take another turn. Sometimes the CPU gets to fill the board here;
        // that should be detected too.
        // @TODO@ -- implement zobrist here!
        return player_sign * evalPosition(board);
    }

    for(const Move &move : moves) {
        Board board_after_move(board);
        makeMove(board_after_move, move);
        int score = -negamax_impl(board_after_move, depth - 1, -beta, -alpha, -player_sign, nodes_searched);
        if(score >= beta) {
            zv.lower_bound = score;
            return score;
        } else if(score > alpha) {
            alpha = score;
            zv.lower_bound = score;
            zv.upper_bound = score;
        }
    }

    zv.upper_bound = alpha;
    return alpha;
}

// @TODO@ -- Assumes AI is PLAYER2
// @TODO@ -- Does not account for draws (default rules don't allow them anyway)
int evalPosition(const Board& board)
{
    int num_pieces_p1 = board.countPieces(PLAYER1);
    int num_pieces_p2 = board.countPieces(PLAYER2);
    // @TODO@ -- assumes board has NUM_SQUARES empty squares
    // (won't be true if we allow squares into which it is illegal to move)
    if(num_pieces_p1 == 0 || num_pieces_p2 == 0 || num_pieces_p1 + num_pieces_p2 == NUM_SQUARES) {
        if(num_pieces_p2 > num_pieces_p1) {
            return WIN;
        } else {
            return LOSS;
        }
    }
    return num_pieces_p2 - num_pieces_p1;
}


// This function does not return all legal moves. For example, if eight
// pieces can clone into the same square, only one move is generated
// since they will have the same result.
void findMoves(const Board& board, Player who, vector<Move>& moves)
{
    findLegalClones(board, who, moves);
    findLegalJumps(board, who, moves);
}

void findLegalClones(const Board& board, Player who, vector<Move>& moves)
{
    for(int y = 0; y < BOARD_SIZE; ++y) {
        for(int x = 0; x < BOARD_SIZE; ++x) {
            if(board(x, y) == EMPTY_SQUARE) {
                appendCloneIfFound(board, x, y, who, moves);
            }
        }
    }
}

// (dst_x, dst_y) is an empty square. It searches surrounding squares
// for a piece occupied by 'who' and appends the move to the list if
// found.
void appendCloneIfFound(const Board& board, int dst_x, int dst_y, Player who, vector<Move>& moves)
{
    for(int src_y = dst_y - 1; src_y <= dst_y + 1; ++src_y) {
        for(int src_x = dst_x - 1; src_x <= dst_x + 1; ++src_x) {
            if(board.isInRange(Coord(src_x, src_y)) && board(src_x, src_y) == who) {
                Move move = {Coord(src_x, src_y), Coord(dst_x, dst_y)};
                moves.push_back(move);
                return;
            }
        }
    }
}

void findLegalJumps(const Board& board, Player who, vector<Move>& moves)
{
    for(int y = 0; y < BOARD_SIZE; ++y) {
        for(int x = 0; x < BOARD_SIZE; ++x) {
            if(board(x, y) == who) {
                appendLegalJumps(board, x, y, moves);
            }
        }
    }
}

// (src_x, src_y) are the location of a piece.
void appendLegalJumps(const Board& board, int src_x, int src_y, vector<Move>& moves)
{
    /*
    (old ordering)
    // Legal jump moves form the perimeter of a square.
    // Handle the left and right sides
    for(int dst_y = src_y - 2; dst_y <= src_y + 2; ++dst_y) {
        appendJumpIfLegal(board, src_x, src_y, src_x - 2, dst_y, moves);
        appendJumpIfLegal(board, src_x, src_y, src_x + 2, dst_y, moves);
    }
    // Handle top and bottom, minus what we've covered already
    // (hence src_x - 1 and + 1 rather than - 2 and + 2)
    for(int dst_x = src_x - 1; dst_x <= src_x + 1; ++dst_x) {
        appendJumpIfLegal(board, src_x, src_y, dst_x, src_y - 2, moves);
        appendJumpIfLegal(board, src_x, src_y, dst_x, src_y + 2, moves);
    }
    */

    for(Coord coord : JUMP_COORDS) {
        appendJumpIfLegal(board, src_x, src_y, src_x + coord.x, src_y + coord.y, moves);
    }
}

void appendJumpIfLegal(const Board& board, int src_x, int src_y, int dst_x, int dst_y, vector<Move>& moves)
{
    if(board.isInRange(Coord(dst_x, dst_y)) && board(dst_x, dst_y) == EMPTY_SQUARE) {
        Move move = {Coord(src_x, src_y), Coord(dst_x, dst_y)};
        moves.push_back(move);
    }
}

}
