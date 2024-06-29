#include <schess/move.h>

static inline void
bitboard_set(square sq, bitboard *b) { *b |= sq2bb(sq); }

static inline void
bitboard_unset(square sq, bitboard *b) { *b &= ~sq2bb(sq); }


// TODO: move to eval.c
int
eval_piece_value(piece_type type)
{
  const int piece_values[PT_COUNT] = { 0, 1, 3, 3, 5, 9, 100, -1, -3, -3, -5, -9, -100 };
  return piece_values[type];
};


int
move_make(move *m, board_state *board, irreversable_state *meta)
{
  int value;
  piece_type piece = board->types[m->from],
  capture = board->types[m->to];

  value = eval_piece_value(capture);

  // clear board
  bitboard_unset(m->from, &board->bitboards[piece]);
  bitboard_unset(m->to, &board->bitboards[capture]);
  board->types[m->from] = PT_NONE;

  if (capture != PT_NONE) meta->halfmove_clock = 0;

  piece_type promo_type, castle_rook;

  switch (m->type)
  {
  case MT_NORMAL:
    if (piece == PT_WP || piece == PT_BP) meta->halfmove_clock = 0;
    break;
  case MT_DOUBLE_PAWN:
    meta->en_passant_potential = sq2bb(m->to) & 0x000000FFFF000000;
    meta->halfmove_clock = 0;
    break;
  case MT_EN_PASSANT:
    if (piece == PT_WP)
    {
      bitboard_unset(m->to + 8, &board->bitboards[PT_BP]);
      value += eval_piece_value(PT_BP);
    }
    else // piece = PT_BP
    {
      bitboard_unset(m->to - 8, &board->bitboards[PT_WP]);
      value += eval_piece_value(PT_WP);
    }
    meta->halfmove_clock = 0;
    break;

  case MT_CASTLE_KING:
    castle_rook = piece + (PR_R - PR_K);
    bitboard_unset(m->from | 0x3, &board->bitboards[castle_rook]);
    bitboard_set(m->to - 1, &board->bitboards[castle_rook]);
    break;
  case MT_CASTLE_QUEEN:
    castle_rook = piece + (PR_R - PR_K);
    bitboard_unset(m->from & ~0x3, &board->bitboards[castle_rook]);
    bitboard_set(m->to + 1, &board->bitboards[castle_rook]);
    break;

#define MOVE_MAKE_HANDLE_PROMOTION(rel_type) \
    { \
      promo_type = piece + (rel_type - PR_P); \
      bitboard_set(m->to, &board->bitboards[promo_type]); \
      value += eval_piece_value(promo_type) - eval_piece_value(piece); \
      meta->halfmove_clock = 0; \
      piece = promo_type; \
    }
  case MT_PROMOTION_KNIGHT: MOVE_MAKE_HANDLE_PROMOTION(PR_N); break;
  case MT_PROMOTION_BISHOP: MOVE_MAKE_HANDLE_PROMOTION(PR_B); break;
  case MT_PROMOTION_ROOK: MOVE_MAKE_HANDLE_PROMOTION(PR_R); break;
  case MT_PROMOTION_QUEEN: MOVE_MAKE_HANDLE_PROMOTION(PR_Q); break;
#undef MOVE_MAKE_HANDLE_PROMOTION
  }


  // unset castle rights if rook is taken
  switch (m->to)
  {
  case 0x07: meta->castling_rights &= 0xFF0000000000000F; break;
  case 0x00: meta->castling_rights &= 0xFF000000000000F0; break;
  case 0x3F: meta->castling_rights &= 0x0F000000000000FF; break;
  case 0x37: meta->castling_rights &= 0xF0000000000000FF; break;
  default: break;
  }

  switch (m->from)
  {
  case 0x07: meta->castling_rights &= 0xFF0000000000000F; break; // white rook kingside moved
  case 0x00: meta->castling_rights &= 0xFF000000000000F0; break; // white rook queenside moved
  case 0x3F: meta->castling_rights &= 0x0F000000000000FF; break; // black rook kingside moved
  case 0x37: meta->castling_rights &= 0xF0000000000000FF; break; // black rook queenside moved
  case 0x04: meta->castling_rights &= 0xFF00000000000000; break; // white king moved
  case 0x3C: meta->castling_rights &= 0x00000000000000FF; break; // black king moved
  default: break;
  }

  // set board
  bitboard_set(m->to, &board->bitboards[piece]);
  board->types[m->to] = piece;

  return value;
}


int
move_unmake(move *m, board_state *board)
{
  int value;
  piece_type piece = board->types[m->to],
  capture = m->capture;

  value = eval_piece_value(capture);

  bitboard_unset(m->to, &board->bitboards[piece]);
  bitboard_set(m->to, &board->bitboards[capture]);
  board->types[m->to] = capture;

  piece_type prepromo_type, castle_rook;

  switch (m->type)
  {
  case MT_NORMAL:
  case MT_DOUBLE_PAWN:
    break;
  case MT_EN_PASSANT:
    if (piece == PT_WP)
    {
      bitboard_set(m->to + 8, &board->bitboards[PT_BP]);
      value += eval_piece_value(PT_BP);
    }
    else // piece = PT_BP
    {
      bitboard_set(m->to - 8, &board->bitboards[PT_WP]);
      value += eval_piece_value(PT_WP);
    }
    break;

  case MT_CASTLE_KING:
    castle_rook = piece + (PR_R - PR_K);
    bitboard_unset(m->to - 1, &board->bitboards[castle_rook]);
    bitboard_set(m->from | 0x3, &board->bitboards[castle_rook]); // TODO: probably wrong
    break;
  case MT_CASTLE_QUEEN:
    castle_rook = piece + (PR_R - PR_K);
    bitboard_unset(m->to + 1, &board->bitboards[castle_rook]);
    bitboard_set(m->from & ~0x3, &board->bitboards[castle_rook]); // TODO: probably wrong
    break;

#define MOVE_UNMAKE_HANDLE_PROMOTION(rel_type) \
    { \
      prepromo_type = piece + (PR_P - rel_type); \
      bitboard_unset(m->to, &board->bitboards[piece]); \
      value += eval_piece_value(prepromo_type) - eval_piece_value(piece); \
      piece = prepromo_type; \
    }
  case MT_PROMOTION_KNIGHT: MOVE_UNMAKE_HANDLE_PROMOTION(PR_N); break;
  case MT_PROMOTION_BISHOP: MOVE_UNMAKE_HANDLE_PROMOTION(PR_B); break;
  case MT_PROMOTION_ROOK: MOVE_UNMAKE_HANDLE_PROMOTION(PR_R); break;
  case MT_PROMOTION_QUEEN: MOVE_UNMAKE_HANDLE_PROMOTION(PR_Q); break;
#undef MOVE_UNMAKE_HANDLE_PROMOTION
  }

  bitboard_set(m->from, &board->bitboards[piece]);
  board->types[m->from] = piece;

  return -value;
}
