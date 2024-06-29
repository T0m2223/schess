#include <schess/gen.h>
#include <schess/lut.h>
#include <schess/types.h>
#include <stddef.h>
#include <stdlib.h>
#include <strings.h>
#include <x86intrin.h>

static bitboard attack_table[LUT_BISHOP_SIZE + LUT_ROOK_SIZE];

static bitboard rook_mask[NUM_SQUARES];
static size_t rook_offset[NUM_SQUARES];
static inline bitboard
rook_attacks(bitboard occ, square sq)
{
  // X86
  return attack_table[rook_offset[sq] + _pext_u64(occ, rook_mask[sq])];
}

static bitboard bishop_mask[NUM_SQUARES];
static size_t bishop_offset[NUM_SQUARES];
static inline bitboard
bishop_attacks(bitboard occ, square sq)
{
  // X86
  return attack_table[bishop_offset[sq] + _pext_u64(occ, bishop_mask[sq])];
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
  square first = ffsll(*board) - 1;
  *board ^= sq2bb(first);
  return first;
}
// TODO: rewrite
static inline square
log_bit(bitboard board)
{
  // TODO: danger; wrong word size
  // LINUX
  return ffsll(board) - 1;
}


static inline void
move_buffer_append_move(square from, square to, piece_type capture, enum MOVE_TYPE type, struct move_buffer *out)
{
  out->moves[out->size].from    = from;
  out->moves[out->size].to      = to;
  out->moves[out->size].capture = capture;
  out->moves[out->size].type    = type;
  ++out->size;
}

static inline void
move_buffer_append_attacks(bitboard attacks, square from, piece_type types[NUM_SQUARES], struct move_buffer *out)
{
  square to;

  while (attacks)
  {
    to = pop_bit(&attacks);

    move_buffer_append_move(from, to, types[to], MT_NORMAL, out);
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
  (void) other;
  bitboard attacks = knight_attacks[sq] & ~own;
  move_buffer_append_attacks(attacks, sq, types, out);
}

// Does not check for 'checked by enemy king'
static inline int
is_square_checked(bitboard own, bitboard other, bitboard other_pieces[6], bitboard other_pawn_attacks, square sq)
{
  bitboard attacker;
  bitboard occ = own | other;

  attacker  = rook_attacks(occ, sq)   & (other_pieces[PR_R] | other_pieces[PR_Q]);
  attacker |= bishop_attacks(occ, sq) & (other_pieces[PR_B] | other_pieces[PR_Q]);
  attacker |= knight_attacks[sq]      & (other_pieces[PR_N]);
  attacker |= other_pawn_attacks      & sq2bb(sq);

  return attacker != 0;
}

static bitboard king_attacks[NUM_SQUARES];
static inline void
generate_king_moves(bitboard own, bitboard other, bitboard other_pieces[6], bitboard other_pawn_attacks, square sq, piece_type types[NUM_SQUARES], irreversable_state meta, struct move_buffer *out)
{
  bitboard occ = own | other;
  bitboard attacks = king_attacks[sq] & ~own;
  move_buffer_append_attacks(attacks, sq, types, out);

  // is king checked?
  if (is_square_checked(own, other, other_pieces, other_pawn_attacks, sq)) return;

  // castling moves
  bitboard castle_east = sq2bb(sq + 2),
           castle_west = castle_east >> 4;
  if (castle_east & meta.castling_rights &&
      !is_square_checked(own, other, other_pieces, other_pawn_attacks, sq + 1) &&
      !(occ & (sq2bb(sq + 1) | sq2bb(sq + 2))))
  {
    move_buffer_append_move(sq, sq + 2, PT_NONE, MT_CASTLE_KING, out);
  }
  if (castle_west & meta.castling_rights &&
      !is_square_checked(own, other, other_pieces, other_pawn_attacks, sq - 1) &&
      !(occ & (sq2bb(sq - 1) | sq2bb(sq - 2) | sq2bb(sq - 3))))
  {
    move_buffer_append_move(sq, sq - 2, PT_NONE, MT_CASTLE_QUEEN, out);
  }
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
  move_buffer_append_move(from, to, capture, MT_PROMOTION_KNIGHT, out);
  move_buffer_append_move(from, to, capture, MT_PROMOTION_BISHOP, out);
  move_buffer_append_move(from, to, capture, MT_PROMOTION_ROOK, out);
  move_buffer_append_move(from, to, capture, MT_PROMOTION_QUEEN, out);
}

static inline void
generate_pawn_moves_white(bitboard own, bitboard other, bitboard pieces, piece_type types[NUM_SQUARES], bitboard en_passant_potential, struct move_buffer *out)
{
  bitboard occ = own | other,
           singles = (pieces  << 0x8) & ~occ,
           doubles = (singles << 0x8) & ~occ & rank_4,
           east_captures = (singles << 0x1) & ~a_file & other,
           west_captures = (singles >> 0x1) & ~h_file & other;

  square from, to;

  bitboard en_passant_east = (en_passant_potential >> 0x1) & pieces;
  if (en_passant_east) // en passant east
  {
    from = log_bit(en_passant_potential >> 0x1);
    to   = log_bit(en_passant_potential << 0x8);
    move_buffer_append_move(from, to, PT_NONE, MT_EN_PASSANT, out);
  }
  bitboard en_passant_west = (en_passant_potential << 0x1) & pieces;
  if (en_passant_west) // en passant west
  {
    from = log_bit(en_passant_potential << 0x1);
    to   = log_bit(en_passant_potential << 0x8);
    move_buffer_append_move(from, to, PT_NONE, MT_EN_PASSANT, out);
  }

  while (singles)
  {
    to = pop_bit(&singles);
    from = to - 0x8;
    if (sq2bb(to) & ~rank_8) // no promotion
    {
      move_buffer_append_move(from, to, PT_NONE, MT_NORMAL, out);
    }
    else // promotion
    {
      move_buffer_append_promotions(from, to, PT_NONE, out);
    }
  }
  while (doubles)
  {
    to = pop_bit(&doubles);
    from = to - 0x10;
    move_buffer_append_move(from, to, PT_NONE, MT_DOUBLE_PAWN, out);
  }
  while (east_captures)
  {
    to = pop_bit(&east_captures);
    from = to - 0x9;
    if (sq2bb(to) & ~rank_8) // no promotion
    {
      move_buffer_append_move(from, to, types[to], MT_NORMAL, out);
    }
    else // promotion
    {
      move_buffer_append_promotions(from, to, types[to], out);
    }
  }
  while (west_captures)
  {
    to = pop_bit(&west_captures);
    from = to - 0x7;
    if (sq2bb(to) & ~rank_8) // no promotion
    {
      move_buffer_append_move(from, to, types[to], MT_NORMAL, out);
    }
    else // promotion
    {
      move_buffer_append_promotions(from, to, types[to], out);
    }
  }
}

static inline void
generate_pawn_moves_black(bitboard own, bitboard other, bitboard pieces, piece_type types[NUM_SQUARES], bitboard en_passant_potential, struct move_buffer *out)
{
  bitboard occ = own | other,
           singles = (pieces  >> 0x8) & ~occ,
           doubles = (singles >> 0x8) & ~occ & rank_5,
           east_captures = (singles << 0x1) & ~a_file & other,
           west_captures = (singles >> 0x1) & ~h_file & other;

  square from, to;

  bitboard en_passant_east = (en_passant_potential >> 0x1) & ~a_file & pieces;
  if (en_passant_east) // en passant east
  {
    from = log_bit(en_passant_potential >> 0x1);
    to   = log_bit(en_passant_potential >> 0x8);
    move_buffer_append_move(from, to, PT_NONE, MT_EN_PASSANT, out);
  }
  bitboard en_passant_west = (en_passant_potential << 0x1) & ~h_file & pieces;
  if (en_passant_west) // en passant west
  {
    from = log_bit(en_passant_potential << 0x1);
    to   = log_bit(en_passant_potential >> 0x8);
    move_buffer_append_move(from, to, PT_NONE, MT_EN_PASSANT, out);
  }

  while (singles)
  {
    to = pop_bit(&singles);
    from = to + 0x8;
    if (sq2bb(to) & ~rank_1) // no promotion
    {
      move_buffer_append_move(from, to, PT_NONE, MT_NORMAL, out);
    }
    else // promotion
    {
      move_buffer_append_promotions(from, to, PT_NONE, out);
    }
  }
  while (doubles)
  {
    to = pop_bit(&doubles);
    from = to + 0x10;
    move_buffer_append_move(from, to, PT_NONE, MT_DOUBLE_PAWN, out);
  }
  while (east_captures)
  {
    to = pop_bit(&east_captures);
    from = to + 0x7;
    if (sq2bb(to) & ~rank_1) // no promotion
    {
      move_buffer_append_move(from, to, types[to], MT_NORMAL, out);
    }
    else // promotion
    {
      move_buffer_append_promotions(from, to, types[to], out);
    }
  }
  while (west_captures)
  {
    to = pop_bit(&west_captures);
    from = to + 0x9;
    if (sq2bb(to) & ~rank_1) // no promotion
    {
      move_buffer_append_move(from, to, types[to], MT_NORMAL, out);
    }
    else // promotion
    {
      move_buffer_append_promotions(from, to, types[to], out);
    }
  }
}


size_t
generate_moves(game_state *game, irreversable_state meta, struct move_buffer *out)
{
  board_state *board = &game->board;
  piece_type color_own = game->active,
             color_other = PT_WP + PT_BP - color_own;
  bitboard *own = board->bitboards + color_own,
           *other = board->bitboards + color_other;
  bitboard own_union = own[PR_P] | own[PR_N] | own[PR_B] | own[PR_R] | own[PR_Q] | own[PR_K];
  bitboard other_union = other[PR_P] | other[PR_N] | other[PR_B] | other[PR_R] | other[PR_Q] | other[PR_K];

  bitboard copy;
#define GENERATE_ALL_MOVES(PT, own, other, pieces, types, out) \
  copy = pieces; \
  while (copy) \
  { \
    square from = pop_bit(&copy); \
    generate_## PT ##_moves(own, other, from, types, out); \
  }

  out->size = 0;

  // generate sliding moves
  GENERATE_ALL_MOVES(bishop, own_union, other_union, own[PR_B], board->types, out);
  GENERATE_ALL_MOVES(rook, own_union, other_union, own[PR_R], board->types, out);
  GENERATE_ALL_MOVES(queen, own_union, other_union, own[PR_Q], board->types, out);

  GENERATE_ALL_MOVES(knight, own_union, other_union, own[PR_N], board->types, out);

#undef GENERATE_ALL_MOVES

  bitboard other_pawn_attacks;
  if (color_other + PR_P == PT_BP) // white's move
  {
    generate_pawn_moves_white(own_union, other_union, board->bitboards[PT_WP], board->types, game->en_passant_potential, out);
    other_pawn_attacks  = (other[PR_P] >> 9) & ~h_file;
    other_pawn_attacks |= (other[PR_P] >> 7) & ~a_file;
    generate_king_moves(own_union, other_union, other, other_pawn_attacks, log_bit(own[PR_K]), board->types, meta, out);
  }
  else // black's move
  {
    generate_pawn_moves_black(own_union, other_union, board->bitboards[PT_BP], board->types, game->en_passant_potential, out);
    other_pawn_attacks  = (other[PR_P] << 7) & ~h_file;
    other_pawn_attacks |= (other[PR_P] << 9) & ~a_file;
    generate_king_moves(own_union, other_union, other, other_pawn_attacks, log_bit(own[PR_K]), board->types, meta, out);
  }

  return out->size;
}

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
