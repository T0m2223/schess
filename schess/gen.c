#include <schess/gen.h>
#include <schess/lut.h>
#include <schess/types.h>
#include <stddef.h>
#include <stdlib.h>
#include <strings.h>
#include <x86intrin.h>

static bitboard attack_table[LUT_BISHOP_SIZE + LUT_ROOK_SIZE];

static bitboard rook_mask[NUM_SQUARES];
static unsigned rook_offset[NUM_SQUARES];
static inline bitboard
rook_attacks(bitboard occ, square sq)
{
  // X86
  return attack_table[rook_offset[sq] + _pext_u64(occ, rook_mask[sq])];
}

static bitboard bishop_mask[NUM_SQUARES];
static unsigned bishop_offset[NUM_SQUARES];
static inline bitboard
bishop_attacks(bitboard occ, square sq)
{
  // X86
  return attack_table[bishop_offset[sq] + _pext_u64(occ, rook_mask[sq])];
}

static inline bitboard
queen_attacks(bitboard occ, square sq)
{
  return rook_attacks(occ, sq) | bishop_attacks(occ, sq);
}

// TODO: rewrite
static inline square
pop_bit(bitboard *board)
{
  // TODO: danger; wrong word size
  // LINUX
  square first = ffsl(*board) - 1;
  *board ^= (1 << first);
  return first;
}
// TODO: rewrite
static inline square
log_bit(bitboard board)
{
  // TODO: danger; wrong word size
  // LINUX
  return ffsl(board) - 1;
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

static bitboard knight_attacks[NUM_SQUARES];
static inline void
generate_knight_moves(bitboard own, bitboard other, square sq, piece_type types[NUM_SQUARES], struct move_buffer *out)
{
  bitboard attacks = knight_attacks[sq] & ~own;
  move_buffer_append_attacks(attacks, sq, types, out);
}

static inline int
is_square_checked(bitboard own, bitboard other, bitboard other_pieces[6], square sq, piece_type types[NUM_SQUARES])
{
  bitboard attacker;
  bitboard occ = own | other;

  attacker  = rook_attacks(occ, sq)   & (other_pieces[PR_R] | other_pieces[PR_Q]);
  attacker |= bishop_attacks(occ, sq) & (other_pieces[PR_B] | other_pieces[PR_Q]);
  attacker |= knight_attacks[sq]      & (other_pieces[PR_N]);

  // TODO: Pawn attacks

  return attacker != 0;
}

static bitboard king_attacks[NUM_SQUARES];
static inline void
generate_king_moves(bitboard own, bitboard other, bitboard other_pieces[6], square sq, piece_type types[NUM_SQUARES], irreversable_state meta, struct move_buffer *out)
{
  bitboard attacks = king_attacks[sq] & ~own;
  move_buffer_append_attacks(attacks, sq, types, out);

  // TODO: castling

}

static const bitboard rank_1 = 0x00000000000000FF;
static const bitboard rank_4 = 0x00000000FF000000;
static const bitboard rank_5 = 0x000000FF00000000;
static const bitboard rank_8 = 0xFF00000000000000;
static const bitboard a_file = 0x0101010101010101;
static const bitboard h_file = 0x8080808080808080;

static inline void
move_buffer_append_promotions(square from, square to, piece_type capture, struct move_buffer *out)
{
  out->moves[out->size].from    = from;
  out->moves[out->size].to      = to;
  out->moves[out->size].capture = capture;
  out->moves[out->size].type    = MT_PROMOTION_KNIGHT;
  ++out->size;
  out->moves[out->size].from    = from;
  out->moves[out->size].to      = to;
  out->moves[out->size].capture = capture;
  out->moves[out->size].type    = MT_PROMOTION_BISHOP;
  ++out->size;
  out->moves[out->size].from    = from;
  out->moves[out->size].to      = to;
  out->moves[out->size].capture = capture;
  out->moves[out->size].type    = MT_PROMOTION_ROOK;
  ++out->size;
  out->moves[out->size].from    = from;
  out->moves[out->size].to      = to;
  out->moves[out->size].capture = capture;
  out->moves[out->size].type    = MT_PROMOTION_QUEEN;
  ++out->size;
}

static inline void
generate_pawn_moves_white(bitboard own, bitboard other, bitboard pieces, piece_type types[NUM_SQUARES], irreversable_state meta, struct move_buffer *out)
{
  bitboard occ = own | other,
           singles = (pieces  << 0x8) & ~occ,
           doubles = (singles << 0x8) & ~occ & rank_4,
           east_captures = (singles << 0x1) & ~a_file & other,
           west_captures = (singles >> 0x1) & ~h_file & other;

  square from, to;

  bitboard en_passant_east = (meta.en_passant_potential >> 0x1) & pieces;
  if (en_passant_east) // en passant east
  {
    out->moves[out->size].from    = log_bit(meta.en_passant_potential >> 0x1);
    out->moves[out->size].to      = log_bit(meta.en_passant_potential << 0x8);
    out->moves[out->size].capture = PT_NONE;
    out->moves[out->size].type    = MT_EN_PASSANT;
    ++out->size;
  }
  bitboard en_passant_west = (meta.en_passant_potential << 0x1) & pieces;
  if (en_passant_west) // en passant west
  {
    out->moves[out->size].from    = log_bit(meta.en_passant_potential << 0x1);
    out->moves[out->size].to      = log_bit(meta.en_passant_potential << 0x8);
    out->moves[out->size].capture = PT_NONE;
    out->moves[out->size].type    = MT_EN_PASSANT;
    ++out->size;
  }

  while (singles)
  {
    to = pop_bit(&singles);
    from = to >> 0x8;
    if (to & ~rank_8) // no promotion
    {
      out->moves[out->size].from = from;
      out->moves[out->size].to = to;
      out->moves[out->size].capture = PT_NONE;
      out->moves[out->size].type = MT_NORMAL;
      ++out->size;
    } else // promotion
    {
      move_buffer_append_promotions(from, to, PT_NONE, out);
    }
  }
  while (doubles)
  {
    to = pop_bit(&doubles);
    from = to >> 0x10;
    out->moves[out->size].from = from;
    out->moves[out->size].to = to;
    out->moves[out->size].capture = PT_NONE;
    out->moves[out->size].type = MT_DOUBLE_PAWN;
    ++out->size;
  }
  while (east_captures)
  {
    to = pop_bit(&east_captures);
    from = to >> 0x9;
    if (to & ~rank_8) // no promotion
    {
      out->moves[out->size].from = from;
      out->moves[out->size].to = to;
      out->moves[out->size].capture = types[to];
      out->moves[out->size].type = MT_NORMAL;
      ++out->size;
    } else // promotion
    {
      move_buffer_append_promotions(from, to, types[to], out);
    }
  }
  while (west_captures)
  {
    to = pop_bit(&west_captures);
    from = to >> 0x7;
    if (to & ~rank_8) // no promotion
    {
      out->moves[out->size].from = from;
      out->moves[out->size].to = to;
      out->moves[out->size].capture = types[to];
      out->moves[out->size].type = MT_NORMAL;
      ++out->size;
    } else // promotion
    {
      move_buffer_append_promotions(from, to, types[to], out);
    }
  }
}

static inline void
generate_pawn_moves_black(bitboard own, bitboard other, bitboard pieces, piece_type types[NUM_SQUARES], irreversable_state meta, struct move_buffer *out)
{
  bitboard occ = own | other,
           singles = (pieces  >> 0x8) & ~occ,
           doubles = (singles >> 0x8) & ~occ & rank_4,
           east_captures = (singles << 0x1) & ~a_file & other,
           west_captures = (singles >> 0x1) & ~h_file & other;

  square from, to;

  bitboard en_passant_east = (meta.en_passant_potential >> 0x1) & pieces;
  if (en_passant_east) // en passant east
  {
    out->moves[out->size].from    = log_bit(meta.en_passant_potential >> 0x1);
    out->moves[out->size].to      = log_bit(meta.en_passant_potential >> 0x8);
    out->moves[out->size].capture = PT_NONE;
    out->moves[out->size].type    = MT_EN_PASSANT;
    ++out->size;
  }
  bitboard en_passant_west = (meta.en_passant_potential << 0x1) & pieces;
  if (en_passant_west) // en passant west
  {
    out->moves[out->size].from    = log_bit(meta.en_passant_potential << 0x1);
    out->moves[out->size].to      = log_bit(meta.en_passant_potential >> 0x8);
    out->moves[out->size].capture = PT_NONE;
    out->moves[out->size].type    = MT_EN_PASSANT;
    ++out->size;
  }

  while (singles)
  {
    to = pop_bit(&singles);
    from = to << 0x8;
    if (to & ~rank_1) // no promotion
    {
      out->moves[out->size].from = from;
      out->moves[out->size].to = to;
      out->moves[out->size].capture = PT_NONE;
      out->moves[out->size].type = MT_NORMAL;
      ++out->size;
    } else // promotion
    {
      move_buffer_append_promotions(from, to, PT_NONE, out);
    }
  }
  while (doubles)
  {
    to = pop_bit(&doubles);
    from = to << 0x10;
    out->moves[out->size].from = from;
    out->moves[out->size].to = to;
    out->moves[out->size].capture = PT_NONE;
    out->moves[out->size].type = MT_DOUBLE_PAWN;
    ++out->size;
  }
  while (east_captures)
  {
    to = pop_bit(&east_captures);
    from = to << 0x7;
    if (to & ~rank_1) // no promotion
    {
      out->moves[out->size].from = from;
      out->moves[out->size].to = to;
      out->moves[out->size].capture = types[to];
      out->moves[out->size].type = MT_NORMAL;
      ++out->size;
    } else // promotion
    {
      move_buffer_append_promotions(from, to, types[to], out);
    }
  }
  while (west_captures)
  {
    to = pop_bit(&west_captures);
    from = to << 0x9;
    if (to & ~rank_1) // no promotion
    {
      out->moves[out->size].from = from;
      out->moves[out->size].to = to;
      out->moves[out->size].capture = types[to];
      out->moves[out->size].type = MT_NORMAL;
      ++out->size;
    } else // promotion
    {
      move_buffer_append_promotions(from, to, types[to], out);
    }
  }
}


#define GENERATE_ALL_MOVES(PT, own, other, pieces, types, out) \
  while (pieces) \
{ \
  square from = pop_bit(&pieces); \
  generate_## PT ##_moves(own, other, from, types, out); \
}

size_t
generate_moves(board_state *board, irreversable_state meta, piece_type color_own, piece_type color_other, struct move_buffer *out)
{
  bitboard *own = board->bitboards + color_own,
           *other = board->bitboards + color_other;
  bitboard own_union = own[PR_P] | own[PR_N] | own[PR_B] | own[PR_R] | own[PR_Q] | own[PR_K];
  bitboard other_union = other[PR_P] | other[PR_N] | other[PR_B] | other[PR_R] | other[PR_Q] | other[PR_K];

  // generate sliding moves
  GENERATE_ALL_MOVES(bishop, own_union, other_union, own[PR_B], board->types, out);
  GENERATE_ALL_MOVES(rook, own_union, other_union, own[PR_R], board->types, out);
  GENERATE_ALL_MOVES(queen, own_union, other_union, own[PR_Q], board->types, out);

  GENERATE_ALL_MOVES(knight, own_union, other_union, own[PR_N], board->types, out);

  if (color_other + PR_P == PT_BP) // white pawn moves
    generate_pawn_moves_white(own_union, other_union, board->bitboards[PT_WP], board->types, meta, out);
  else // black pawn moves
    generate_pawn_moves_black(own_union, other_union, board->bitboards[PT_BP], board->types, meta, out);

  return out->size;
}

#undef GENERATE_ALL_MOVES

struct move_buffer *
move_buffer_create(size_t max_ply)
{
  return calloc(max_ply, sizeof(struct move_buffer));
}

void
move_gen_init_LUTs(void)
{
  lut_gen_knight(knight_attacks);
  lut_gen_bishop_rook(attack_table, bishop_mask, rook_mask, bishop_offset, rook_offset);
  lut_gen_king(king_attacks);
}
