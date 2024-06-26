#include <schess/gen.h>
#include <schess/move.h>
#include <schess/types.h>
#include <stddef.h>

int quiesce(int alpha, int beta)
{
  return 1;
}

int
alpha_beta_min(board_state *board, irreversable_state meta, int alpha, int beta, unsigned depth, struct move_buffer *mbuf);
int
alpha_beta_min(board_state *board, irreversable_state meta, int alpha, int beta, unsigned depth, struct move_buffer *mbuf) { return 1; }

int
alpha_beta_max(board_state *board, irreversable_state meta, int alpha, int beta, unsigned depth, struct move_buffer *mbuf)
{
  if (!depth) return quiesce(alpha, beta);

  size_t i, num_moves;
  int score;
  irreversable_state meta_copy;

  num_moves = generate_moves(&board->bitboards[PT_WP], &board->bitboards[PT_BP], board->types, meta, &mbuf[depth]);

  for (i = 0; i < num_moves; ++i)
  {
    // move ordering
    move *m = mbuf[depth].moves + i;

    meta_copy = meta;
    move_make(m, board, &meta_copy);
    score = -alpha_beta_min(board, meta_copy, alpha, beta, depth - 1, mbuf);
    move_unmake(m, board);

    if (score >= beta) return beta;
    if (score > alpha) alpha = score;
  }
  return alpha;
}

