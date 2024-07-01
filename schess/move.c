#include <schess/eval.h>
#include <schess/move.h>

static inline void
bitboard_set(square sq, bitboard *b) { *b |= sq2bb(sq); }

static inline void
bitboard_unset(square sq, bitboard *b) { *b &= ~sq2bb(sq); }


void
move_make(move *m, game_state *game, irreversable_state *meta)
{
  board_state *board = &game->board;
  piece_type piece = board->types[m->from],
  capture = board->types[m->to];

  // clear board
  bitboard_unset(m->from, &board->bitboards[piece]);
  bitboard_unset(m->to, &board->bitboards[capture]);
  board->types[m->from] = PT_NONE;

  if (capture != PT_NONE) meta->halfmove_clock = 0;

  game->en_passant_potential = 0ull;

  piece_type promo_type, castle_rook;

  switch (m->type)
  {
  case MT_NORMAL:
    if (piece == PT_WP || piece == PT_BP) meta->halfmove_clock = 0;
    break;
  case MT_DOUBLE_PAWN:
    game->en_passant_potential = sq2bb(m->to);
    meta->halfmove_clock = 0;
    break;
  case MT_EN_PASSANT:
    if (piece == PT_WP)
    {
      bitboard_unset(m->to - 8, &board->bitboards[PT_BP]);
      board->types[m->to - 8] = PT_NONE;
    }
    else // piece = PT_BP
    {
      bitboard_unset(m->to + 8, &board->bitboards[PT_WP]);
      board->types[m->to + 8] = PT_NONE;
    }
    meta->halfmove_clock = 0;
    break;

  case MT_CASTLE_KING:
    castle_rook = piece + (PR_R - PR_K);
    bitboard_unset(m->to + 1, &board->bitboards[castle_rook]);
    bitboard_set(m->from + 1, &board->bitboards[castle_rook]);
    board->types[m->to + 1] = PT_NONE;
    board->types[m->from + 1] = castle_rook;
    break;
  case MT_CASTLE_QUEEN:
    castle_rook = piece + (PR_R - PR_K);
    bitboard_unset(m->to - 2, &board->bitboards[castle_rook]);
    bitboard_set(m->from - 1, &board->bitboards[castle_rook]);
    board->types[m->to - 2] = PT_NONE;
    board->types[m->from - 1] = castle_rook;
    break;

#define MOVE_MAKE_HANDLE_PROMOTION(rel_type) \
    { \
      promo_type = piece + (rel_type - PR_P); \
      meta->halfmove_clock = 0; \
      piece = promo_type; \
    }
  case MT_PROMOTION_KNIGHT: MOVE_MAKE_HANDLE_PROMOTION(PR_N); break;
  case MT_PROMOTION_BISHOP: MOVE_MAKE_HANDLE_PROMOTION(PR_B); break;
  case MT_PROMOTION_ROOK: MOVE_MAKE_HANDLE_PROMOTION(PR_R); break;
  case MT_PROMOTION_QUEEN: MOVE_MAKE_HANDLE_PROMOTION(PR_Q); break;
#undef MOVE_MAKE_HANDLE_PROMOTION

  case MT_NULL: break;
  }


  // unset castle rights if rook is taken
  switch (m->to)
  {
  case h1: meta->castling_rights &= 0xFF0000000000000F; break;
  case a1: meta->castling_rights &= 0xFF000000000000F0; break;
  case h8: meta->castling_rights &= 0x0F000000000000FF; break;
  case a8: meta->castling_rights &= 0xF0000000000000FF; break;
  default: break;
  }

  switch (m->from)
  {
  case h1: meta->castling_rights &= 0xFF0000000000000F; break; // white rook kingside moved
  case a1: meta->castling_rights &= 0xFF000000000000F0; break; // white rook queenside moved
  case h8: meta->castling_rights &= 0x0F000000000000FF; break; // black rook kingside moved
  case a8: meta->castling_rights &= 0xF0000000000000FF; break; // black rook queenside moved
  case e1: meta->castling_rights &= 0xFF00000000000000; break; // white king moved
  case e8: meta->castling_rights &= 0x00000000000000FF; break; // black king moved
  default: break;
  }

  // set board
  bitboard_set(m->to, &board->bitboards[piece]);
  board->types[m->to] = piece;

  game->active = OTHER_COLOR(game->active);
}


void
move_unmake(move *m, game_state *game)
{
  board_state *board = &game->board;
  piece_type piece = board->types[m->to],
  capture = m->capture;

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
      bitboard_set(m->to - 8, &board->bitboards[PT_BP]);
      board->types[m->to - 8] = PT_BP;
    }
    else // piece = PT_BP
    {
      bitboard_set(m->to + 8, &board->bitboards[PT_WP]);
      board->types[m->to + 8] = PT_WP;
    }
    break;

  case MT_CASTLE_KING:
    castle_rook = piece + (PR_R - PR_K);
    bitboard_unset(m->from + 1, &board->bitboards[castle_rook]);
    bitboard_set(m->to + 1, &board->bitboards[castle_rook]);
    board->types[m->to + 1] = castle_rook;
    board->types[m->from + 1] = PT_NONE;
    break;
  case MT_CASTLE_QUEEN:
    castle_rook = piece + (PR_R - PR_K);
    bitboard_unset(m->from - 1, &board->bitboards[castle_rook]);
    bitboard_set(m->to - 2, &board->bitboards[castle_rook]);
    board->types[m->to - 2] = castle_rook;
    board->types[m->from - 1] = PT_NONE;
    break;

#define MOVE_UNMAKE_HANDLE_PROMOTION(rel_type) \
    { \
      prepromo_type = piece + (PR_P - rel_type); \
      piece = prepromo_type; \
    }
  case MT_PROMOTION_KNIGHT: MOVE_UNMAKE_HANDLE_PROMOTION(PR_N); break;
  case MT_PROMOTION_BISHOP: MOVE_UNMAKE_HANDLE_PROMOTION(PR_B); break;
  case MT_PROMOTION_ROOK: MOVE_UNMAKE_HANDLE_PROMOTION(PR_R); break;
  case MT_PROMOTION_QUEEN: MOVE_UNMAKE_HANDLE_PROMOTION(PR_Q); break;
#undef MOVE_UNMAKE_HANDLE_PROMOTION

  case MT_NULL: break;
  }

  bitboard_set(m->from, &board->bitboards[piece]);
  board->types[m->from] = piece;

  game->active = OTHER_COLOR(game->active);
}
