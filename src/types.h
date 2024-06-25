#ifndef SCHESS_TYPES_H
#define SCHESS_TYPES_H
#include <stdint.h>

#define NUM_SQUARES 64

/* BOARD STATE */
typedef uint64_t bitboard;
enum PIECE_REL { PR_P, PR_N, PR_B, PR_R, PR_Q, PR_K };
enum PIECE_TYPE
{
  PT_NONE,
  PT_WP, PT_WN, PT_WB, PT_WR, PT_WQ, PT_WK,
  PT_BP, PT_BN, PT_BB, PT_BR, PT_BQ, PT_BK,
  PT_COUNT
};
typedef struct
{
  bitboard bitboards[PT_COUNT];
  enum PIECE_TYPE types[NUM_SQUARES];
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
  unsigned from, to;
  enum PIECE_TYPE capture;
  enum MOVE_TYPE type;
} move;


/* IRREVERSABLE STATE */
typedef struct
{
  unsigned halfmove_clock;
  bitboard en_passant_castling_rights;
} irreversable_state;


#endif // SCHESS_TYPES_H
