#ifndef SCHESS_UTILS_H
#define SCHESS_UTILS_H
#include <schess/gen.h>
#include <schess/types.h>

static const char *square_names[NUM_SQUARES] =
{
  "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
  "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
  "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
  "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
  "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
  "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
  "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
  "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
};

static const char *piece_names[PT_COUNT] =
{
  "  ",
  "WP", "WN", "WB", "WR", "WQ", "WK",
  "BP", "BN", "BB", "BR", "BQ", "BK",
};

static const char *move_names[9] =
{
  "NORMAL",
  "DOUBLE_PAWN",
  "EN_PASSANT",
  "CASTLE_KING",
  "CASTLE_QUEEN",
  "PROMOTION_KNIGHT",
  "PROMOTION_BISHOP",
  "PROMOTION_ROOK",
  "PROMOTION_QUEEN",
};

int
parse_FEN(const char *FEN, board_state *board, irreversable_state *meta);

void
print_board(board_state *board);

void
print_moves(board_state *board, struct move_buffer *mbuf);

#endif // SCHESS_UTILS_H
