#include <schess/eval.h>
#include <schess/gen.h>
#include <schess/move.h>
#include <schess/search.h>
#include <schess/utils.h>
#include <stddef.h>

int quiesce(game_state *game, irreversable_state meta, int alpha, int beta)
{
  (void) alpha;
  (void) beta;

  int res = eval_position(game, meta);

  return res;
}

int
alpha_beta(game_state *game, irreversable_state meta, int alpha, int beta, unsigned depth, struct move_buffer *mbuf)
{
  if (!depth) return quiesce(game, meta, alpha, beta);
  depth -= 1;

  size_t i, num_moves;
  int score = 42;
  irreversable_state meta_copy;

  num_moves = generate_moves(game, meta, &mbuf[depth]);

  for (i = 0; i < num_moves; ++i)
  {
    move *m = mbuf[depth].moves + i;
    meta_copy = meta;

    move_make(m, game, &meta_copy);
    score = -alpha_beta(game, meta_copy, -beta, -alpha, depth, mbuf);
    move_unmake(m, game);

    if (score >= beta) return beta;
    if (score > alpha) alpha = score;
  }
  return alpha;
}

move
search_best_move(game_state *game, irreversable_state meta, unsigned depth)
{
  size_t i, num_moves;
  struct move_buffer *mbuf = move_buffer_create(depth);
  int score, best_score = -oo;
  move best = { 0 };
  irreversable_state meta_copy;

  if (depth == 0) return (move) { .type = MT_NULL };
  depth -= 1;

  num_moves = generate_moves(game, meta, &mbuf[depth]);
  for (i = 0; i < num_moves; ++i)
  {
    move *m = mbuf[depth].moves + i;
    meta_copy = meta;

    move_make(m, game, &meta_copy);
    score = -alpha_beta(game, meta_copy, -oo, +oo, depth, mbuf);
    move_unmake(m, game);


    if (score > best_score)
    {
      best_score = score;
      best = *m;
    }
  }

  return best;
}
