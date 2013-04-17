#include <algorithm>
#include <vector>

#include "Board.hpp"
#include "moves.hpp"
#include "ai.hpp"
#include "bitboards.hpp"
#include "zobrist.hpp"

namespace
{

int negamax_bb_root(const Board& board, int depth, int alpha, int beta, Move& move_out, int& nodes_searched);
int negamax_bb_impl(Bitboard bb_player1, Bitboard bb_player2, int depth, int alpha, int beta, int player_sign, BitboardMove& move_out, int& nodes_searched);
bool searchMove(const BitboardMove bbmove, int& score, ScoreType& score_type,
    Bitboard bb_player1, Bitboard bb_player2, Bitboard bb_me,
    Bitboard bb_him, int depth, int& alpha, int beta, int player_sign,
    BitboardMove& move_out, int& nodes_searched);
void makeMoveBB(BitboardMove bbmove, Bitboard& me, Bitboard& him);
int evalPositionBB(Bitboard player1, Bitboard player2);
void convBoardToBitboards(const Board& board, Bitboard& player1, Bitboard& player2);
void convBitboardMoveToMove(const Board& board, Player player, BitboardMove bb_move, Move& move);
void convSquareNumToCoord(int square_num, Coord& coord);

inline void handleCaptures(Bitboard capture_radius, Bitboard& me, Bitboard& him)
{
    Bitboard captures = him & capture_radius;
    me |= captures;
    him &= ~captures;
}

}

// TRANSPOSITION TABLE MUST BE INITIALIZED BEFORE CALLING
int negamax_bb(const Board& board, Move& move_out, int& nodes_searched)
{
    return negamax_bb_root(board, MAX_PLY, -INFINITY, INFINITY, move_out, nodes_searched);
}

// TRANSPOSITION TABLE MUST BE INITIALIZED BEFORE CALLING
int negamax_iterative_bb(const Board& board, Move& move_out, int& nodes_searched)
{
    int score;
    for(int depth = 1; depth <= MAX_PLY; ++depth) {
        score = negamax_bb_root(board, depth, -INFINITY, INFINITY, move_out, nodes_searched);
    }
    return score;
}

// TRANSPOSITION TABLE MUST BE INITIALIZED BEFORE CALLING
int mtdf_bb(const Board& board, Move& move_out, int& nodes_searched)
{
    int f = 0;
    for(int depth = 1; depth <= MAX_PLY; ++depth) {
        f = mtdf_impl(board, depth, f, move_out, nodes_searched, negamax_bb_root);
    }
    return f;
}


namespace
{

int negamax_bb_root(const Board& board, int depth, int alpha, int beta, Move& move_out, int& nodes_searched)
{
    Bitboard bb_player1, bb_player2;
    convBoardToBitboards(board, bb_player1, bb_player2);
    BitboardMove bb_move_out;
    int score = negamax_bb_impl(bb_player1, bb_player2, depth, alpha, beta, 1, bb_move_out, nodes_searched);
    // @TODO@ -- assumes AI is player 2
    convBitboardMoveToMove(board, PLAYER2, bb_move_out, move_out);
    return score;
}

// player_sign is 1 for Max (i.e. this AI player), -1 for Min
// Throughout, "me" refers to the player whose move we're considering; "him" refers to his opponent
// So "me" alternates between player 1 and player 2 throughout the tree
int negamax_bb_impl(Bitboard bb_player1, Bitboard bb_player2, int depth, int alpha, int beta, int player_sign, BitboardMove& move_out, int& nodes_searched)
{
    ++nodes_searched;

    if(!ENABLE_ALPHA_BETA) {
        alpha = -INFINITY;
        beta = INFINITY;
    }

    if(ENABLE_ZOBRIST) {
        ZobristValue zobrist_value = getZobristValueBB(bb_player1, bb_player2, player_sign);
        if(zobrist_value.depth >= depth) {
            // @TODO@ -- should check if zobrist_value.best_move is a legal move.
            // If it is not, there was a hash collision (or a bug, but hopefully not).
            // In that case, not only can we invalidate the score, but we know not to
            // try playing it as the best move. (In extremely rare cases, unlikely to
            // ever occur in a real game, this could result in the root node of the
            // tree returning an illegal move as the move to play!)
            switch(zobrist_value.score_type) {
              case SCORE_EXACT:
                return zobrist_value.score;

              case SCORE_ALPHA:
                if(zobrist_value.score <= alpha) {
                    return alpha;
                }
                break;

              case SCORE_BETA:
                if(zobrist_value.score >= beta) {
                    return beta;
                }
                break;

              default:
                assert(false);
            }
        }
        // @TODO@ -- move this into its own variable?
        move_out = zobrist_value.best_move;
    } else {
        move_out.move_type = BBMOVE_NONE;
    }

    if(depth == 0) {
        int score = player_sign * evalPositionBB(bb_player1, bb_player2);
        ZobristValue zobrist_value(score, SCORE_EXACT, depth);
        zobrist_value.best_move.move_type = BBMOVE_NONE;
        setZobristValueBB(bb_player1, bb_player2, player_sign, zobrist_value);
        return score;
    }

    // @TODO@ -- all this assumes AI is player 2
    Bitboard bb_pieces = bb_player1 | bb_player2;
    Bitboard bb_empty = invertBitboard(bb_pieces);
    Bitboard bb_me = bb_player1;
    Bitboard bb_him = bb_player2;
    if(player_sign == 1) {
        std::swap(bb_me, bb_him);
    }

    ScoreType score_type = SCORE_ALPHA;
    bool found_any_moves = false;

    // If we found a best move via transposition table, try it now.
    if(BEST_FIRST && move_out.move_type != BBMOVE_NONE) {
        int score;
        if(searchMove(move_out, score, score_type, bb_player1,
                      bb_player2, bb_me, bb_him, depth, alpha,
                      beta, player_sign, move_out,
                      nodes_searched))
        {
            // Beta cutoff
            return score;
        }
    }

    // Try to generate moves
    // If we analyzed a "best move", it will show up again here. We're hoping
    // the transposition table takes care of it.
    std::uint64_t bit = 1;
    for(int square_num = 0; square_num < 7*7; ++square_num) {
        BitboardMove bbmove(BBMOVE_NONE, 0, 0);

        if(bit & bb_empty) {
            // This is an empty square; try to clone into it
            if(bb_me & BITBOARD_SURROUNDS[square_num]) {
                found_any_moves = true;
                bbmove.move_type = BBMOVE_CLONE;
                bbmove.square = square_num;
                int score;
                if(searchMove(bbmove, score, score_type, bb_player1,
                              bb_player2, bb_me, bb_him, depth, alpha,
                              beta, player_sign, move_out,
                              nodes_searched))
                {
                    // Beta cutoff
                    return score;
                }
            }
        } else if(bit & bb_me) {
            // This is one of my squares; try to generate jumps
            const BitboardJump* jumps = BITBOARD_JUMPS[square_num];
            for(int i = 0; i < NUM_JUMPS; ++i) {
                if(jumps[i].dest_square & bb_empty) {
                    found_any_moves = true;
                    bbmove.move_type = BBMOVE_JUMP;
                    bbmove.square = square_num;
                    bbmove.jump_type = i;
                    int score;
                    if(searchMove(bbmove, score, score_type, bb_player1,
                                  bb_player2, bb_me, bb_him, depth, alpha,
                                  beta, player_sign, move_out,
                                  nodes_searched))
                    {
                        // Beta cutoff
                        return score;
                    }
                }
            }
        }

        bit <<= 1;
    }

    if(!found_any_moves) {
        // No moves were found
        // @TODO@ -- This is not correct behavior!
        // Should check if game over and if so return position evaluation.
        // Otherwise, should take another turn. Sometimes the CPU gets to fill the board here;
        // that should be detected too.
        // @TODO@ -- implement zobrist here!
        return player_sign * evalPositionBB(bb_player1, bb_player2);
    }

    ZobristValue zobrist_value(alpha, score_type, depth, move_out);
    setZobristValueBB(bb_player1, bb_player2, player_sign, zobrist_value);
    return alpha;
}

// Returns true if there's been a beta cutoff, false if not
bool searchMove(BitboardMove bbmove, int& score, ScoreType& score_type,
    Bitboard bb_player1, Bitboard bb_player2, Bitboard bb_me,
    Bitboard bb_him, int depth, int& alpha, int beta, int player_sign,
    BitboardMove& move_out, int& nodes_searched)
{
    Bitboard bb_me_after = bb_me;
    Bitboard bb_him_after = bb_him;
    makeMoveBB(bbmove, bb_me_after, bb_him_after);

    Bitboard bb_p1_after = bb_me_after;
    Bitboard bb_p2_after = bb_him_after;
    // @TODO@ assumes AI is player 2
    if(player_sign == 1) {
        std::swap(bb_p1_after, bb_p2_after);
    }
    BitboardMove dummy;     // @TODO@ -- make unnecessary
    score = -negamax_bb_impl(bb_p1_after, bb_p2_after, depth - 1, -beta, -alpha, -player_sign, dummy, nodes_searched);
    if(score >= beta) {
        move_out = bbmove;
        ZobristValue zobrist_value(score, SCORE_BETA, depth, move_out);
        setZobristValueBB(bb_player1, bb_player2, player_sign, zobrist_value);
        return true;
    } else if(score > alpha) {
        alpha = score;
        score_type = SCORE_EXACT;
        move_out = bbmove;
    }
    return false;
}

void makeMoveBB(BitboardMove bbmove, Bitboard& me, Bitboard& him)
{
    Bitboard square_bit = (1LL << bbmove.square);
    switch(bbmove.move_type)
    {
      case BBMOVE_CLONE:
        me |= square_bit;
        handleCaptures(BITBOARD_SURROUNDS[bbmove.square], me, him);
        break;

      case BBMOVE_JUMP:
      {
        BitboardJump jump = BITBOARD_JUMPS[bbmove.square][bbmove.jump_type];
        me |= jump.dest_square;
        me &= invertBitboard(square_bit);      // remove piece from source square
        handleCaptures(jump.capture_radius, me, him);
        break;
      }

      default:
        assert(false);
    }
}

// @TODO@ -- Assumes AI is PLAYER2
// @TODO@ -- Does not account for draws (default rules don't allow them anyway)
int evalPositionBB(Bitboard player1, Bitboard player2)
{
    int num_pieces_p1 = countSetBits(player1);
    int num_pieces_p2 = countSetBits(player2);
    // @TODO@ -- hardcoded board size (the 49)
    if(num_pieces_p1 == 0 || num_pieces_p2 == 0 || num_pieces_p1 + num_pieces_p2 == 49) {
        if(num_pieces_p2 > num_pieces_p1) {
            return INFINITY;
        } else {
            return -INFINITY;
        }
    }
    return num_pieces_p2 - num_pieces_p1;
}

void convBoardToBitboards(const Board& board, Bitboard& player1, Bitboard& player2)
{
    player1 = player2 = 0;
    Bitboard bit = 1;
    for(int y = 0; y < 7; ++y) {
        for(int x = 0; x < 7; ++x) {
            switch(board(x, y)) {
              case EMPTY_SQUARE:    break;
              case PLAYER1:         player1 |= bit; break;
              case PLAYER2:         player2 |= bit; break;
              default:              assert(false);
            }
            bit <<= 1;
        }
    }
}

void convBitboardMoveToMove(const Board& board, Player player, BitboardMove bb_move, Move& move)
{
    switch(bb_move.move_type) {
      case BBMOVE_CLONE:
        convSquareNumToCoord(bb_move.square, move.dst);
        for(int y = move.dst.y - 1; y <= move.dst.y + 1; ++y) {
            for(int x = move.dst.x - 1; x <= move.dst.x + 1; ++x) {
                Coord coord(x, y);
                if(board.isInRange(coord) && board(x, y) == player) {
                    move.src = coord;
                    return;
                }
            }
        }
        // We only get here if a clone move was generated, but no friendly
        // piece neighbors the destination square
        assert(false);

      case BBMOVE_JUMP:
        convSquareNumToCoord(bb_move.square, move.src);
        move.dst.x = move.src.x + JUMP_COORDS[bb_move.jump_type].x;
        move.dst.y = move.src.y + JUMP_COORDS[bb_move.jump_type].y;
        break;

      default:
        assert(false);
    }
}

void convSquareNumToCoord(int square_num, Coord& coord)
{
    coord.x = square_num % 7;
    coord.y = square_num / 7;
}

}