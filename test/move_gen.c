#include <stdio.h>
#include <test/base.h>
#include <schess/gen.h>
#include <schess/move.h>
#include <schess/types.h>
#include <schess/utils.h>
#include <stddef.h>

typedef unsigned long long ull;

int
board_eq(board_state a, board_state b)
{
  size_t i;

  for (i = 1; i < PT_COUNT; ++i)
    if (a.bitboards[i] != b.bitboards[i]) return 0;

  for (i = 0; i < NUM_SQUARES; ++i)
    if (a.types[i] != b.types[i]) return 0;

  return 1;
}

struct perft_result
{
  ull nodes,
      captures,
      en_passants,
      castles,
      promotions;
};

struct perft_result
perft_results_add(struct perft_result a, struct perft_result b)
{
  a.nodes       += b.nodes;
  a.captures    += b.captures;
  a.en_passants += b.en_passants;
  a.castles     += b.castles;
  a.promotions  += b.promotions;

  return a;
}

int
perft_results_compare(struct perft_result a, struct perft_result b)
{
  if (a.nodes       != b.nodes      ) return 1;
  if (a.captures    != b.captures   ) return 2;
  if (a.en_passants != b.en_passants) return 3;
  if (a.castles     != b.castles    ) return 4;
  if (a.promotions  != b.promotions ) return 5;

  return 0;
}

static struct perft_result
perft_rec(game_state *game, irreversable_state meta, unsigned depth, struct move_buffer *mbuf)
{
  if (!depth) return (struct perft_result) { .nodes = 1 };

  size_t num_moves, i;
  irreversable_state meta_copy;
  board_state cpy;
  struct perft_result res = { 0 };

  num_moves = generate_moves(game, meta, &mbuf[depth - 1]);
  for (i = 0; i < num_moves; ++i)
  {
    meta_copy = meta;
    move *m = mbuf[depth - 1].moves + i;
    cpy = game->board;
    move_make(m, game, &meta_copy);
    if (is_board_legal(&game->board, game->active))
    {
      res = perft_results_add(res, perft_rec(game, meta_copy, depth - 1, mbuf));
      if (depth == 1)
      {
        if (m->type == MT_CASTLE_KING || m->type == MT_CASTLE_QUEEN) ++res.castles;
        if (m->type == MT_EN_PASSANT) { ++res.en_passants; ++res.captures; }
        if (m->type == MT_PROMOTION_QUEEN || m->type == MT_PROMOTION_BISHOP || m->type == MT_PROMOTION_KNIGHT || m->type == MT_PROMOTION_ROOK) ++res.promotions;
        if (m->capture != PT_NONE) ++res.captures;
      }
    }
    move_unmake(m, game);
    if (!board_eq(cpy, game->board)) fprintf(stderr, "Fatal board diff: %s\n", move_name(m->type));
  }

  return res;
}

static struct perft_result
perft(game_state *game, irreversable_state meta, unsigned depth)
{
  struct move_buffer *mbuf = move_buffer_create(depth);

  return perft_rec(game, meta, depth, mbuf);
}


TEST(kiwipete_perft)
{
  size_t i;
  game_state game;
  irreversable_state meta;
  const size_t depth = 7;
  const struct perft_result expected[] =
  {
    { .nodes = 1          , .captures = 0          , .en_passants = 0       , .castles = 0         , .promotions = 0        },
    { .nodes = 48         , .captures = 8          , .en_passants = 0       , .castles = 2         , .promotions = 0        },
    { .nodes = 2039       , .captures = 351        , .en_passants = 1       , .castles = 91        , .promotions = 0        },
    { .nodes = 97862      , .captures = 17102      , .en_passants = 45      , .castles = 3162      , .promotions = 0        },
    { .nodes = 4085603    , .captures = 757163     , .en_passants = 1929    , .castles = 128013    , .promotions = 15172    },
    { .nodes = 193690690  , .captures = 35043416   , .en_passants = 73365   , .castles = 4993637   , .promotions = 8392     },
    { .nodes = 8031647685 , .captures = 1558445089 , .en_passants = 3577504 , .castles = 184513607 , .promotions = 56627920 },
  };
  struct perft_result res;

  move_gen_init_LUTs();

  for (i = 0; i < depth; ++i)
  {
    parse_FEN("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", &game, &meta);
    res = perft(&game, meta, i);

    int diff = perft_results_compare(res, expected[i]);
    if (diff)
      return diff;
  }

  return 0;
}


TEST(starting_pos_perft)
{
  size_t i;
  game_state game;
  irreversable_state meta;
  const size_t depth = 8;
  const struct perft_result expected[] =
  {
    { .nodes = 1          , .captures = 0         , .en_passants = 0      , .castles = 0      , .promotions = 0 },
    { .nodes = 20         , .captures = 0         , .en_passants = 0      , .castles = 0      , .promotions = 0 },
    { .nodes = 400        , .captures = 0         , .en_passants = 0      , .castles = 0      , .promotions = 0 },
    { .nodes = 8902       , .captures = 34        , .en_passants = 0      , .castles = 0      , .promotions = 0 },
    { .nodes = 197281     , .captures = 1576      , .en_passants = 0      , .castles = 0      , .promotions = 0 },
    { .nodes = 4865609    , .captures = 82719     , .en_passants = 258    , .castles = 0      , .promotions = 0 },
    { .nodes = 119060324  , .captures = 2812008   , .en_passants = 5248   , .castles = 0      , .promotions = 0 },
    { .nodes = 3195901860 , .captures = 108329926 , .en_passants = 319617 , .castles = 883453 , .promotions = 0 },
  };
  struct perft_result res;

  move_gen_init_LUTs();

  for (i = 0; i < depth; ++i)
  {
    parse_FEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", &game, &meta);
    res = perft(&game, meta, i);

    int diff = perft_results_compare(res, expected[i]);
    if (diff)
      return diff;
  }

  return 0;
}


TEST(position3_perft)
{
  size_t i;
  game_state game;
  irreversable_state meta;
  const size_t depth = 9;
  const struct perft_result expected[] =
  {
    { .nodes = 1          , .captures = 0         , .en_passants = 0       , .castles = 0 , .promotions = 0       },
    { .nodes = 14         , .captures = 1         , .en_passants = 0       , .castles = 0 , .promotions = 0       },
    { .nodes = 191        , .captures = 14        , .en_passants = 0       , .castles = 0 , .promotions = 0       },
    { .nodes = 2812       , .captures = 209       , .en_passants = 2       , .castles = 0 , .promotions = 0       },
    { .nodes = 43238      , .captures = 3348      , .en_passants = 123     , .castles = 0 , .promotions = 0       },
    { .nodes = 674624     , .captures = 52051     , .en_passants = 1165    , .castles = 0 , .promotions = 0       },
    { .nodes = 11030083   , .captures = 940350    , .en_passants = 33325   , .castles = 0 , .promotions = 7552    },
    { .nodes = 178633661  , .captures = 14519036  , .en_passants = 294874  , .castles = 0 , .promotions = 140024  },
    { .nodes = 3009794393 , .captures = 267586558 , .en_passants = 8009239 , .castles = 0 , .promotions = 6578076 },
  };
  struct perft_result res;

  move_gen_init_LUTs();

  for (i = 0; i < depth; ++i)
  {
    parse_FEN("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1", &game, &meta);
    res = perft(&game, meta, i);

    int diff = perft_results_compare(res, expected[i]);
    if (diff)
      return diff;
  }

  return 0;
}


TEST(position4_perft)
{
  size_t i;
  game_state game;
  irreversable_state meta;
  const size_t depth = 7;
  const struct perft_result expected[] =
  {
    { .nodes = 1         , .captures = 0         , .en_passants = 0      , .castles = 0        , .promotions = 0        },
    { .nodes = 6         , .captures = 0         , .en_passants = 0      , .castles = 0        , .promotions = 0        },
    { .nodes = 264       , .captures = 87        , .en_passants = 0      , .castles = 6        , .promotions = 48       },
    { .nodes = 9467      , .captures = 1021      , .en_passants = 4      , .castles = 0        , .promotions = 120      },
    { .nodes = 422333    , .captures = 131393    , .en_passants = 0      , .castles = 7795     , .promotions = 60032    },
    { .nodes = 15833292  , .captures = 2046173   , .en_passants = 6512   , .castles = 0        , .promotions = 329464   },
    { .nodes = 706045033 , .captures = 210369132 , .en_passants = 212    , .castles = 10882006 , .promotions = 81102984 },
  };
  struct perft_result res;

  move_gen_init_LUTs();

  for (i = 0; i < depth; ++i)
  {
    parse_FEN("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1", &game, &meta);
    res = perft(&game, meta, i);

    int diff = perft_results_compare(res, expected[i]);
    if (diff)
      return diff;
  }

  return 0;
}


TEST(position5_perft)
{
  size_t i;
  game_state game;
  irreversable_state meta;
  const size_t depth = 6;
  const struct perft_result expected[] =
  {
    { .nodes = 1        },
    { .nodes = 44       },
    { .nodes = 1486     },
    { .nodes = 62379    },
    { .nodes = 2103487  },
    { .nodes = 89941194 },
  };
  struct perft_result res;

  move_gen_init_LUTs();

  for (i = 0; i < depth; ++i)
  {
    parse_FEN("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8", &game, &meta);
    res = perft(&game, meta, i);

    ull diff = res.nodes - expected[i].nodes;
    if (diff)
      return diff;
  }

  return 0;
}


TEST(position6_perft)
{
  size_t i;
  game_state game;
  irreversable_state meta;
  const size_t depth = 7; // TODO: maybe change to 10
  const struct perft_result expected[] =
  {
    { .nodes = 1               },
    { .nodes = 46              },
    { .nodes = 2079            },
    { .nodes = 89890           },
    { .nodes = 3894594         },
    { .nodes = 164075551       },
    { .nodes = 6923051137      },
    { .nodes = 287188994746    },
    { .nodes = 11923589843526  },
    { .nodes = 490154852788714 },
  };
  struct perft_result res;

  move_gen_init_LUTs();

  for (i = 0; i < depth; ++i)
  {
    parse_FEN("r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10", &game, &meta);
    res = perft(&game, meta, i);

    ull diff = res.nodes - expected[i].nodes;
    if (diff)
      return diff;
  }

  return 0;
}

