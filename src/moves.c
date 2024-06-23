#include <stddef.h>
#include "types.h"

static inline bitboard
bitboard_set(bitboard b, size_t idx) { return b | (1 << idx); }

static inline bitboard
bitboard_unset(bitboard b, size_t idx) { return b & ~(1 << idx); }


// TODO: move to eval.c
int
eval_piece_value(enum PIECE_TYPE type)
{
  const int piece_values[13] = { 0, 1, 3, 3, 5, 9, 100, -1, -3, -3, -5, -9, -100 };
  return piece_values[type];
};

// TODO: set irreversable_state attributes
int
move_make(move *m, board_state *board, irreversable_state *meta)
{
  int value;
  enum PIECE_TYPE piece = board->types[m->from],
  capture = board->types[m->to];

  value = eval_piece_value(capture);

  // clear board
  bitboard_unset(board->bitboards[piece], m->from);
  bitboard_unset(board->bitboards[capture], m->to);
  board->types[m->from] = PT_NONE;

  if (piece == PT_WP || piece == PT_BP || capture != PT_NONE) meta->halfmove_clock = 0;
  enum PIECE_TYPE promo_type, castle_rook;

  switch (m->type)
  {
  case MT_NORMAL:
    break;
  case MT_EN_PASSANT:
    if (piece == PT_WP)
    {
      bitboard_unset(board->bitboards[PT_BP], m->to + 8);
      value += eval_piece_value(PT_BP);
    } else // piece = PT_BP
    {
      bitboard_unset(board->bitboards[PT_WP], m->to - 8);
      value += eval_piece_value(PT_WP);
    }
    break;

  case MT_CASTLE_KING:
    castle_rook = piece + (PR_R - PR_K);
    bitboard_unset(board->bitboards[castle_rook], m->from | 0x3);
    bitboard_set(board->bitboards[castle_rook], m->to - 1);
    break;
  case MT_CASTLE_QUEEN:
    castle_rook = piece + (PR_R - PR_K);
    bitboard_unset(board->bitboards[castle_rook], m->from & ~0x3);
    bitboard_set(board->bitboards[castle_rook], m->to + 1);
    break;

#define MOVE_MAKE_HANDLE_PROMOTION(rel_type) \
    { \
      promo_type = piece + (rel_type - PR_P); \
      bitboard_set(board->bitboards[promo_type], m->to); \
      value += eval_piece_value(promo_type) - eval_piece_value(piece); \
      piece = promo_type; \
    }
  case MT_PROMOTION_KNIGHT: MOVE_MAKE_HANDLE_PROMOTION(PR_N); break;
  case MT_PROMOTION_BISHOP: MOVE_MAKE_HANDLE_PROMOTION(PR_B); break;
  case MT_PROMOTION_ROOK: MOVE_MAKE_HANDLE_PROMOTION(PR_R); break;
  case MT_PROMOTION_QUEEN: MOVE_MAKE_HANDLE_PROMOTION(PR_Q); break;
#undef MOVE_MAKE_HANDLE_PROMOTION
  }

  // set board
  bitboard_set(board->bitboards[piece], m->to);
  board->types[m->to] = piece;

  return value;
}


int
move_unmake(move *m, board_state *board, irreversable_state *meta)
{
  int value;
  enum PIECE_TYPE piece = board->types[m->to],
  capture = m->capture;

  value = eval_piece_value(capture);

  bitboard_unset(board->bitboards[piece], m->to);
  bitboard_set(board->bitboards[capture], m->to);
  board->types[m->to] = capture;

  enum PIECE_TYPE prepromo_type, castle_rook;

  switch (m->type)
  {
  case MT_NORMAL:
    break;
  case MT_EN_PASSANT:
    if (piece == PT_WP)
    {
      bitboard_set(board->bitboards[PT_BP], m->to + 8);
      value += eval_piece_value(PT_BP);
    } else // piece = PT_BP
    {
      bitboard_set(board->bitboards[PT_WP], m->to - 8);
      value += eval_piece_value(PT_WP);
    }
    break;

  case MT_CASTLE_KING:
    castle_rook = piece + (PR_R - PR_K);
    bitboard_unset(board->bitboards[castle_rook], m->to - 1);
    bitboard_set(board->bitboards[castle_rook], m->from | 0x3);
    break;
  case MT_CASTLE_QUEEN:
    castle_rook = piece + (PR_R - PR_K);
    bitboard_unset(board->bitboards[castle_rook], m->to + 1);
    bitboard_set(board->bitboards[castle_rook], m->from & ~0x3);
    break;

#define MOVE_UNMAKE_HANDLE_PROMOTION(rel_type) \
    { \
      prepromo_type = piece + (PR_P - rel_type); \
      bitboard_unset(board->bitboards[piece], m->to); \
      value += eval_piece_value(prepromo_type) - eval_piece_value(piece); \
      piece = prepromo_type; \
    }
  case MT_PROMOTION_KNIGHT: MOVE_UNMAKE_HANDLE_PROMOTION(PR_N); break;
  case MT_PROMOTION_BISHOP: MOVE_UNMAKE_HANDLE_PROMOTION(PR_B); break;
  case MT_PROMOTION_ROOK: MOVE_UNMAKE_HANDLE_PROMOTION(PR_R); break;
  case MT_PROMOTION_QUEEN: MOVE_UNMAKE_HANDLE_PROMOTION(PR_Q); break;
#undef MOVE_UNMAKE_HANDLE_PROMOTION
  }

  bitboard_set(board->bitboards[piece], m->from);
  board->types[m->from] = piece;

  return -value;
}
