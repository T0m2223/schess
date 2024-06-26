#include <schess/gen.h>
#include <schess/types.h>
#include <stddef.h>
#include <stdlib.h>
#include <strings.h>
#include <x86intrin.h>


// TODO: table fill
bitboard attack_table[88316];

// TODO: table fill
static bitboard rook_mask[NUM_SQUARES];
static bitboard rook_offset[NUM_SQUARES];
static inline bitboard
rook_attacks(bitboard occ, square sq)
{
  return attack_table[rook_offset[sq] + _pext_u64(occ, rook_mask[sq])];
}

// TODO: table fill
static bitboard bishop_mask[NUM_SQUARES];
static bitboard bishop_offset[NUM_SQUARES];
static inline bitboard
bishop_attacks(bitboard occ, square sq)
{
  return attack_table[bishop_offset[sq] + _pext_u64(occ, rook_mask[sq])];
}

static inline bitboard
queen_attacks(bitboard occ, square sq)
{
  return rook_attacks(occ, sq) | bishop_attacks(occ, sq);
}

// TODO: rewrite
static inline bitboard
pop_bit(bitboard *board)
{
  // TODO: danger; wrong word size
  square first = ffsl(*board) - 1;
  *board ^= (1 << first);
  return first;
}


static inline void
move_buffer_append_attacks(bitboard attacks, square from, piece_type types[NUM_SQUARES], struct move_buffer *out)
{
  while (attacks)
  {
    square to = pop_bit(&attacks);
    out->moves[out->size].from = from;
    out->moves[out->size].to = to;
    out->moves[out->size].capture = types[to];
    out->moves[out->size].type = MT_NORMAL;
    ++out->size;
  }
}

static inline void
generate_rook_moves(bitboard own, bitboard other, square sq, piece_type types[NUM_SQUARES], struct move_buffer *out)
{
  bitboard occ = own | other;
  bitboard attacks = rook_attacks(occ, sq) & ~own;
  move_buffer_append_attacks(attacks, sq, types, out);
}
static inline void
generate_bishop_moves(bitboard own, bitboard other, square sq, piece_type types[NUM_SQUARES], struct move_buffer *out)
{
  bitboard occ = own | other;
  bitboard attacks = bishop_attacks(occ, sq) & ~own;
  move_buffer_append_attacks(attacks, sq, types, out);
}
static inline void
generate_queen_moves(bitboard own, bitboard other, square sq, piece_type types[NUM_SQUARES], struct move_buffer *out)
{
  bitboard occ = own | other;
  bitboard attacks = queen_attacks(occ, sq) & ~own;
  move_buffer_append_attacks(attacks, sq, types, out);
}

// TODO: table fill
bitboard knight_attacks[NUM_SQUARES];
static inline void
generate_knight_moves(bitboard own, bitboard other, square sq, piece_type types[NUM_SQUARES], struct move_buffer *out)
{
  bitboard attacks = knight_attacks[sq] & ~own;
  move_buffer_append_attacks(attacks, sq, types, out);
}


// TODO: table fill
bitboard king_attacks[NUM_SQUARES];
static inline void
generate_king_moves(bitboard own, bitboard other, square sq, piece_type types[NUM_SQUARES], irreversable_state meta, struct move_buffer *out)
{
  bitboard attacks = king_attacks[sq] & ~own;
  move_buffer_append_attacks(attacks, sq, types, out);

  // castling
}

#define GENERATE_ALL_MOVES(PT, own, other, pieces, types, out) \
  while (pieces) \
{ \
  square from = pop_bit(&pieces); \
  generate_## PT ## _moves(own, other, from, types, out); \
}

size_t
generate_moves(bitboard own[6], bitboard other[6], piece_type types[64], irreversable_state meta, struct move_buffer *out)
{
  bitboard own_union = own[PR_P] | own[PR_N] | own[PR_B] | own[PR_R] | own[PR_Q] | own[PR_K];
  bitboard other_union = other[PR_P] | other[PR_N] | other[PR_B] | other[PR_R] | other[PR_Q] | other[PR_K];

  // generate sliding moves
  GENERATE_ALL_MOVES(bishop, own_union, other_union, own[PR_B], types, out);
  GENERATE_ALL_MOVES(rook, own_union, other_union, own[PR_R], types, out);
  GENERATE_ALL_MOVES(queen, own_union, other_union, own[PR_Q], types, out);

  GENERATE_ALL_MOVES(knight, own_union, other_union, own[PR_N], types, out);

  return out->size;
}

#undef GENERATE_ALL_MOVES

struct move_buffer *
move_buffer_create(size_t max_ply)
{
  return calloc(max_ply, sizeof(struct move_buffer));
}
