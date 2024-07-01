#ifndef SCHESS_TYPES_H
#define SCHESS_TYPES_H

#include <limits.h>
#include <stdint.h>

#define oo INT_MAX

enum PIECE_REL { PR_P, PR_N, PR_B, PR_R, PR_Q, PR_K };
typedef enum
{
  PT_NONE,
  PT_WP, PT_WN, PT_WB, PT_WR, PT_WQ, PT_WK,
  PT_BP, PT_BN, PT_BB, PT_BR, PT_BQ, PT_BK,
  PT_COUNT
} piece_type;
#define OTHER_COLOR(color) (COLOR_WHITE + COLOR_BLACK - (color))
typedef enum
{
  COLOR_WHITE = PT_WP - PR_P,
  COLOR_BLACK = PT_BP - PR_P,
} color;

/* BOARD STATE */
typedef uint64_t bitboard;
typedef enum
{
  a1, b1, c1, d1, e1, f1, g1, h1,
  a2, b2, c2, d2, e2, f2, g2, h2,
  a3, b3, c3, d3, e3, f3, g3, h3,
  a4, b4, c4, d4, e4, f4, g4, h4,
  a5, b5, c5, d5, e5, f5, g5, h5,
  a6, b6, c6, d6, e6, f6, g6, h6,
  a7, b7, c7, d7, e7, f7, g7, h7,
  a8, b8, c8, d8, e8, f8, g8, h8, NUM_SQUARES
} square;
typedef struct
{
  bitboard bitboards[PT_COUNT];
  piece_type types[NUM_SQUARES];
} board_state;

static inline bitboard
sq2bb(square sq) { return (1ull << sq); }


/* MOVE */
enum MOVE_TYPE
{
  MT_NORMAL,
  MT_DOUBLE_PAWN,
  MT_EN_PASSANT,
  MT_CASTLE_KING,
  MT_CASTLE_QUEEN,
  MT_PROMOTION_KNIGHT,
  MT_PROMOTION_BISHOP,
  MT_PROMOTION_ROOK,
  MT_PROMOTION_QUEEN,
  MT_NULL,
};
typedef struct
{
  square from, to;
  piece_type capture;
  enum MOVE_TYPE type;
} move;


/* IRREVERSABLE STATE */
typedef struct
{
  unsigned halfmove_clock;
  bitboard castling_rights;
} irreversable_state;


/* GAME STATE */
typedef struct
{
  board_state board;
  bitboard en_passant_potential;
  color active;
  unsigned fullmove;
} game_state;


#endif // SCHESS_TYPES_H
