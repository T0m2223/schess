#include <schess/types.h>
#include <x86intrin.h>


/* KNIGHT LOOK UP TABLE */
static const bitboard not_a_file  = 0xfefefefefefefefe;
static const bitboard not_ab_file = 0xfcfcfcfcfcfcfcfc;
static const bitboard not_h_file  = 0x7f7f7f7f7f7f7f7f;
static const bitboard not_gh_file = 0x3f3f3f3f3f3f3f3f;
static inline bitboard nonoea(bitboard b) { return (b << 17) & not_a_file ; }
static inline bitboard noeaea(bitboard b) { return (b << 10) & not_ab_file; }
static inline bitboard soeaea(bitboard b) { return (b >>  6) & not_ab_file; }
static inline bitboard sosoea(bitboard b) { return (b >> 15) & not_a_file ; }
static inline bitboard nonowe(bitboard b) { return (b << 15) & not_h_file ; }
static inline bitboard nowewe(bitboard b) { return (b <<  6) & not_gh_file; }
static inline bitboard sowewe(bitboard b) { return (b >> 10) & not_gh_file; }
static inline bitboard sosowe(bitboard b) { return (b >> 17) & not_h_file ; }

void
lut_fill_knight_attacks(bitboard lut[NUM_SQUARES])
{
  square sq;
  bitboard knight;

  for (sq = a1; sq < NUM_SQUARES; ++sq)
  {
    knight = (1 << sq);
    lut[sq] = 0;
    lut[sq] |= nonoea(knight);
    lut[sq] |= noeaea(knight);
    lut[sq] |= soeaea(knight);
    lut[sq] |= sosoea(knight);
    lut[sq] |= nonowe(knight);
    lut[sq] |= nowewe(knight);
    lut[sq] |= sowewe(knight);
    lut[sq] |= sosowe(knight);
  }
}


/* ROOK LOOK UP TABLE */
static const bitboard file_attack = 0x0101010101010101;
static const bitboard rank_attack = 0x00000000000000FF;
static const bitboard not_8_rank  = 0x00FFFFFFFFFFFFFF;
static const bitboard not_1_rank  = 0xFFFFFFFFFFFFFF00;
static inline bitboard no(bitboard b) { return (b << 8) & not_8_rank; }
static inline bitboard so(bitboard b) { return (b >> 8) & not_1_rank; }
static inline bitboard ea(bitboard b) { return (b << 1) & not_h_file; }
static inline bitboard we(bitboard b) { return (b >> 1) & not_a_file; }

static inline bitboard
lut_calc_rook_attacks(bitboard occ, square sq)
{
  bitboard rook;
  bitboard attacks = 0;

#define LUT_CALC_ROOK_ATTACKS_DIR(dir) do \
  { \
    rook = 1 << sq; \
    while (rook) \
    { \
      rook = dir(rook); \
      attacks |= rook; \
      rook &= ~occ; \
    } \
  } while (0)

  LUT_CALC_ROOK_ATTACKS_DIR(no);
  LUT_CALC_ROOK_ATTACKS_DIR(so);
  LUT_CALC_ROOK_ATTACKS_DIR(ea);
  LUT_CALC_ROOK_ATTACKS_DIR(we);

#undef LUT_CALC_ROOK_ATTACKS_DIR

  return attacks;
}

unsigned
lut_fill_rook_attacks(bitboard mask[NUM_SQUARES], unsigned offset[NUM_SQUARES], unsigned initial_offset, bitboard lut[])
{
  square sq;
  unsigned file, rank, current_offset, num_entries;
  bitboard board;

  current_offset = initial_offset;

  for (sq = a1; sq < NUM_SQUARES; ++sq)
  {
    offset[sq] = current_offset;

    file = sq & 0x7;
    rank = sq >> 0x3;

    mask[sq]  = file_attack << file;
    mask[sq] |= rank_attack << rank;
    mask[sq] &= ~(1 << sq);

    // GCC
    num_entries = __builtin_popcount(3);

    for (board = 0; board < (1 << num_entries); ++board)
    {
      // X86
      lut[current_offset++] = lut_calc_rook_attacks(_pdep_u64(board, mask[sq]), sq);
    }
  }

  return current_offset;
}


/* BISHOP LOOK UP TABLE */

unsigned
lut_fill_bishop_attacks(bitboard mask[NUM_SQUARES], unsigned offset[NUM_SQUARES], unsigned initial_offset, bitboard lut[])
{

}
