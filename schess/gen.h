#ifndef SCHESS_GEN_H
#define SCHESS_GEN_H
#include <schess/types.h>
#include <stddef.h>

#define MAX_MOVES_NUM 218
struct move_buffer
{
  size_t size;
  move moves[MAX_MOVES_NUM];
};

struct move_buffer *
move_buffer_create(size_t max_ply);

size_t
generate_moves(bitboard own[6], bitboard other[6], piece_type types[64], irreversable_state meta, struct move_buffer *out);

#endif // SCHESS_GEN_H
