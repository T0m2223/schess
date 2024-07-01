#include <schess/eval.h>

int
eval_piece_value(piece_type type)
{
  const int piece_values[PT_COUNT] =
  {
    [PT_NONE] = 0,
    [PT_WP]   = 1,
    [PT_WN]   = 3,
    [PT_WB]   = 3,
    [PT_WR]   = 5,
    [PT_WQ]   = 9,
    [PT_WK]   = oo,
    [PT_BP]   = -1,
    [PT_BN]   = -3,
    [PT_BB]   = -3,
    [PT_BR]   = -5,
    [PT_BQ]   = -9,
    [PT_BK]   = -oo,
  };

  return piece_values[type];
};
