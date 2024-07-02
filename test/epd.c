#include <schess/utils.h>
#include <string.h>
#include <test/base.h>
#include <schess/search.h>
#include <schess/types.h>
#include <stddef.h>


#define EPD_DEFAULT_DEPTH 6

static int
epd_test(const char *epd, unsigned depth)
{
  game_state game;
  irreversable_state meta;
  move best;
  size_t i, whitespaces;

  square from, to;
  piece_type promotion;
  char epd_copy[strlen(epd) + 1];
  int err;

  for (i = 0, whitespaces = 0; i < strlen(epd) && whitespaces < 4; ++i)
  {
    if (epd[i] == ' ')
      ++whitespaces;
  }

  if (whitespaces < 4)
    return 1;

  strcpy(epd_copy, epd);
  epd_copy[i] = '\0';

  err = parse_FEN(epd_copy, &game, &meta);
  if (err) return err;

  if (strlen(&epd[i + 1]) < 3)
    return 1;

  err = parse_SAN(&epd[i + 3], &game, meta, &from, &to, &promotion);
  if (err) return err;

  best = search_best_move(&game, meta, depth);

  if (!strncmp(&epd[i + 1], "bm ", 3))
  {
    if ((promotion == PT_WN || promotion == PT_BN) && best.type != MT_PROMOTION_KNIGHT) return 1;
    if ((promotion == PT_WB || promotion == PT_BB) && best.type != MT_PROMOTION_BISHOP) return 1;
    if ((promotion == PT_WR || promotion == PT_BR) && best.type != MT_PROMOTION_ROOK  ) return 1;
    if ((promotion == PT_WQ || promotion == PT_BQ) && best.type != MT_PROMOTION_QUEEN ) return 1;
    if (best.from != from || best.to != to) return 1;
    return 0;
  }
  else if (!strncmp(&epd[i + 1], "am ", 3))
  {
    if (best.from != from || best.to != to) return 0;
    if ((promotion == PT_WN || promotion == PT_BN) && best.type == MT_PROMOTION_KNIGHT) return 1;
    if ((promotion == PT_WB || promotion == PT_BB) && best.type == MT_PROMOTION_BISHOP) return 1;
    if ((promotion == PT_WR || promotion == PT_BR) && best.type == MT_PROMOTION_ROOK  ) return 1;
    if ((promotion == PT_WQ || promotion == PT_BQ) && best.type == MT_PROMOTION_QUEEN ) return 1;
    return 0;
  }

  return 1;
}


TEST(bk_test)
{
  size_t i, num_positions = 24;
  const char *positions[] =
  {
    "1k1r4/pp1b1R2/3q2pp/4p3/2B5/4Q3/PPP2B2/2K5 b - - bm Qd1+",
    "3r1k2/4npp1/1ppr3p/p6P/P2PPPP1/1NR5/5K2/2R5 w - - bm d5",
    "2q1rr1k/3bbnnp/p2p1pp1/2pPp3/PpP1P1P1/1P2BNNP/2BQ1PRK/7R b - - bm f5",
    "rnbqkb1r/p3pppp/1p6/2ppP3/3N4/2P5/PPP1QPPP/R1B1KB1R w KQkq - bm e6",
    "r1b2rk1/2q1b1pp/p2ppn2/1p6/3QP3/1BN1B3/PPP3PP/R4RK1 w - - bm Nd5 a4",
    "2r3k1/pppR1pp1/4p3/4P1P1/5P2/1P4K1/P1P5/8 w - - bm g6",
    "1nk1r1r1/pp2n1pp/4p3/q2pPp1N/b1pP1P2/B1P2R2/2P1B1PP/R2Q2K1 w - - bm Nf6",
    "4b3/p3kp2/6p1/3pP2p/2pP1P2/4K1P1/P3N2P/8 w - - bm f5",
    "2kr1bnr/pbpq4/2n1pp2/3p3p/3P1P1B/2N2N1Q/PPP3PP/2KR1B1R w - - bm f5",
    "3rr1k1/pp3pp1/1qn2np1/8/3p4/PP1R1P2/2P1NQPP/R1B3K1 b - - bm Ne5",
    "2r1nrk1/p2q1ppp/bp1p4/n1pPp3/P1P1P3/2PBB1N1/4QPPP/R4RK1 w - - bm f4",
    "r3r1k1/ppqb1ppp/8/4p1NQ/8/2P5/PP3PPP/R3R1K1 b - - bm Bf5",
    "r2q1rk1/4bppp/p2p4/2pP4/3pP3/3Q4/PP1B1PPP/R3R1K1 w - - bm b4",
    "rnb2r1k/pp2p2p/2pp2p1/q2P1p2/8/1Pb2NP1/PB2PPBP/R2Q1RK1 w - - bm Qd2 Qe1",
    "2r3k1/1p2q1pp/2b1pr2/p1pp4/6Q1/1P1PP1R1/P1PN2PP/5RK1 w - - bm Qxg7+",
    "r1bqkb1r/4npp1/p1p4p/1p1pP1B1/8/1B6/PPPN1PPP/R2Q1RK1 w kq - bm Ne4",
    "r2q1rk1/1ppnbppp/p2p1nb1/3Pp3/2P1P1P1/2N2N1P/PPB1QP2/R1B2RK1 b - - bm h5",
    "r1bq1rk1/pp2ppbp/2np2p1/2n5/P3PP2/N1P2N2/1PB3PP/R1B1QRK1 b - - bm Nb3",
    "3rr3/2pq2pk/p2p1pnp/8/2QBPP2/1P6/P5PP/4RRK1 b - - bm Rxe4",
    "r4k2/pb2bp1r/1p1qp2p/3pNp2/3P1P2/2N3P1/PPP1Q2P/2KRR3 w - - bm g4",
    "3rn2k/ppb2rpp/2ppqp2/5N2/2P1P3/1P5Q/PB3PPP/3RR1K1 w - - bm Nh6",
    "2r2rk1/1bqnbpp1/1p1ppn1p/pP6/N1P1P3/P2B1N1P/1B2QPP1/R2R2K1 b - - bm Bxe4",
    "r1bqk2r/pp2bppp/2p5/3pP3/P2Q1P2/2N1B3/1PP3PP/R4RK1 b kq - bm f6",
    "r2qnrnk/p2b2b1/1p1p2pp/2pPpp2/1PP1P3/PRNBB3/3QNPPP/5RK1 w - - bm f4",
  };

  for (i = 0; i < num_positions; ++i)
  {
    if (epd_test(positions[i], EPD_DEFAULT_DEPTH))
      return i + 1;
  }

  return 0;
}

