#include <schess/utils.h>
#include <string.h>
#include <test/base.h>
#include <schess/search.h>
#include <schess/types.h>
#include <stddef.h>

int
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
    if (epd[i] == ' ' && ++whitespaces == 4)
      break;
  }

  if (whitespaces < 4)
    return 1;

  strcpy(epd_copy, epd);
  epd_copy[i] = '\0';

  err = parse_FEN(epd_copy, &game, &meta);
  if (err) return err;

  if (strlen(&epd[i + 1]) < 3)
    return 1;

  err = parse_SAN(&epd[i + 4], &game, meta, &from, &to, &promotion);
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
