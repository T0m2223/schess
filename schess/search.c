#include <schess/gen.h>
#include <schess/move.h>
#include <schess/types.h>
#include <stddef.h>

int quiesce(int alpha, int beta)
{
  return 0;
}

int
alpha_beta(game_state *game, irreversable_state meta, int alpha, int beta, unsigned depth, struct move_buffer *mbuf)
{
  if (!depth) return quiesce(alpha, beta);

  size_t i, num_moves;
  int score;
  irreversable_state meta_copy;

  num_moves = generate_moves(game, meta, &mbuf[depth]);

  for (i = 0; i < num_moves; ++i)
  {
    // move ordering
    move *m = mbuf[depth].moves + i;
    meta_copy = meta;

    move_make(m, game, &meta_copy);
    score = -alpha_beta(game, meta_copy, -beta, -alpha, depth - 1, mbuf);
    move_unmake(m, game);

    if (score >= beta) return beta;
    if (score > alpha) alpha = score;
  }
  return alpha;
}

