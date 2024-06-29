#include <test/base.h>
#include <schess/gen.h>
#include <schess/types.h>
#include <schess/utils.h>
#include <stddef.h>

int
test_move_gen(const char *FEN, square *from_to, size_t num_moves)
{
  board_state board;
  irreversable_state meta;
  struct move_buffer *mbuf = move_buffer_create(1);
  size_t i, j;
  square from, to;
  int found;

  move_gen_init_LUTs();
  parse_FEN(FEN, &board, &meta);

  generate_moves(&board, meta, PT_WP, PT_BP, mbuf);

  if (mbuf->size != num_moves) return 1;

  for (i = 0; i < num_moves; ++i)
  {
    found = 0;
    from = from_to[i * 2];
    to = from_to[(i * 2) + 1];

    for (j = 0; j < mbuf->size; ++j)
    {
      move m = mbuf->moves[j];
      if (m.from == from && m.to == to)
      {
        found = 1;
        break;
      }
    }

    if (!found) return 2;
  }

  return 0;
}

#define TEST_MOVE_GEN(name, FEN, from_to, num_moves) \
  TEST(name) \
{ \
  return test_move_gen((FEN), (from_to), (num_moves)); \
}

square moves1[] = { a2, a3, a2, a4, b2, b3, b2, b4, c2, c3, c2, c4, d2, d3, d2, d4, e2, e3, e2, e4, f2, f3, f2, f4, g2, g3, g2, g4, h2, h3, h2, h4, b1, a3, b1, c3, g1, f3, g1, h3 };
TEST_MOVE_GEN(starting_pos, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", moves1, sizeof(moves1) / sizeof(square) / 2);

square moves2[] = { a2, a3, a2, a4, b2, b3, b2, b4, c2, c3, c2, c4, d2, d3, d2, d4, f2, f3, f2, f4, g2, g3, g2, g4, h2, h3, h2, h4, b1, a3, b1, c3, g1, f3, g1, h3, d1, e2, d1, d3, d1, d4, d1, d5, d1, d6, d1, d7, d1, d8, d1, c1, d1, b1, d1, a1, d1, e1, e1, e2, e1, e3, e1, e4, e1, e5, e1, e6, e1, e7, e1, e8, e4, e5, e4, d5, e4, c5, e4, b5, e4, a5, e4, f5, e4, g5, e4, h5 };
TEST_MOVE_GEN(e4_played, "rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq e6 0 2", moves2, sizeof(moves2) / sizeof(square) / 2);
