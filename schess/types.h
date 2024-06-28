#ifndef SCHESS_TYPES_H
#define SCHESS_TYPES_H
#include <stdint.h>

/* BOARD STATE */
typedef uint64_t bitboard;
typedef enum
{
  a1, a2, a3, a4, a5, a6, a7, a8,
  b1, b2, b3, b4, b5, b6, b7, b8,
  c1, c2, c3, c4, c5, c6, c7, c8,
  d1, d2, d3, d4, d5, d6, d7, d8,
  e1, e2, e3, e4, e5, e6, e7, e8,
  f1, f2, f3, f4, f5, f6, f7, f8,
  g1, g2, g3, g4, g5, g6, g7, g8,
  h1, h2, h3, h4, h5, h6, h7, h8, NUM_SQUARES
} square;
enum PIECE_REL { PR_P, PR_N, PR_B, PR_R, PR_Q, PR_K };
typedef enum
{
  PT_NONE,
  PT_WP, PT_WN, PT_WB, PT_WR, PT_WQ, PT_WK,
  PT_BP, PT_BN, PT_BB, PT_BR, PT_BQ, PT_BK,
  PT_COUNT
} piece_type;
typedef struct
{
  bitboard bitboards[PT_COUNT];
  piece_type types[NUM_SQUARES];
} board_state;


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
  bitboard en_passant_potential;
} irreversable_state;


#endif // SCHESS_TYPES_H
