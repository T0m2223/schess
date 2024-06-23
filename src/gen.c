#include <stddef.h>
#include <stdlib.h>
#include "types.h"
#include "gen.h"


// Board state -> List of possible moves

#define MAX_MOVE_NUMS 218


// stack for moves? (size: max ply)
// inplace orderer?

struct move_buffer *move_buffer_create(size_t max_ply)
{
  size_t i;
  struct move_buffer *buffers = malloc(max_ply * sizeof(*buffers));
  move *raw = malloc(MAX_MOVE_NUMS * max_ply * sizeof(*raw));

  for (i = 0; i < max_ply; ++i)
  {
    buffers[i].data = raw + (i * MAX_MOVE_NUMS);
    buffers[i].size = 0;
  }

  return buffers;
}

static inline void
generate_rook_moves(bitboard rooks, bitboard own_union, bitboard other_union, struct move_buffer *out)
{

}
