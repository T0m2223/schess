#ifndef SCHESS_GEN_H
#define SCHESS_GEN_H
#include <stddef.h>
#include "types.h"

struct move_buffer
{
  move *data;
  size_t size;
};

struct move_buffer
*move_buffer_create(size_t max_ply);

size_t
generate_moves(bitboard own[6], bitboard other[6], enum PIECE_TYPE types[64], struct move_buffer *out);

#endif // SCHESS_GEN_H
