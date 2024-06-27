#include <schess/lut_gen/lut.h>
#include <x86intrin.h>

static const bitboard file_attack = 0x0101010101010101;
static const bitboard rank_attack = 0x00000000000000FF;
static const bitboard not_a_file = ~0x0101010101010101;
static const bitboard not_h_file = ~0x8080808080808080;

// directional move functions
static inline bitboard no(bitboard b) { return (b << 8); }
static inline bitboard so(bitboard b) { return (b >> 8); }
static inline bitboard ea(bitboard b) { return (b << 1) & not_a_file; }
static inline bitboard we(bitboard b) { return (b >> 1) & not_h_file; }
static inline bitboard noea(bitboard b) { return no(ea(b)); }
static inline bitboard soea(bitboard b) { return so(ea(b)); }
static inline bitboard nowe(bitboard b) { return no(we(b)); }
static inline bitboard sowe(bitboard b) { return so(we(b)); }
static inline bitboard nonoea(bitboard b) { return no(noea(b)); }
static inline bitboard noeaea(bitboard b) { return ea(noea(b)); }
static inline bitboard soeaea(bitboard b) { return ea(soea(b)); }
static inline bitboard sosoea(bitboard b) { return so(soea(b)); }
static inline bitboard nonowe(bitboard b) { return no(nowe(b)); }
static inline bitboard nowewe(bitboard b) { return we(nowe(b)); }
static inline bitboard sowewe(bitboard b) { return we(sowe(b)); }
static inline bitboard sosowe(bitboard b) { return so(sowe(b)); }

/* KNIGHT LOOK UP TABLE */
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
lut_fill_rook_attacks(bitboard mask[NUM_SQUARES], unsigned offset[NUM_SQUARES], unsigned initial_offset, bitboard lut[LUT_ROOK_SIZE])
{
  square sq;
  unsigned file, rank, current_offset, num_entries;
  bitboard board;

  current_offset = initial_offset;

  for (sq = a1; sq < NUM_SQUARES; ++sq)
  {
    offset[sq] = current_offset;

    // generate mask
    file = sq & 0x7;
    rank = sq >> 0x3;
    mask[sq]  = file_attack << file;
    mask[sq] |= rank_attack << rank;
    mask[sq] &= ~(1 << sq);

    // GCC
    num_entries = __builtin_popcount(mask[sq]);

    for (board = 0; board < (1u << num_entries); ++board)
    {
      // X86
      lut[current_offset++] = lut_calc_rook_attacks(_pdep_u64(board, mask[sq]), sq);
    }
  }

  return current_offset;
}


/* BISHOP LOOK UP TABLE */
static inline bitboard
lut_calc_bishop_attacks(bitboard occ, square sq)
{
  bitboard bishop;
  bitboard attacks = 0;

#define LUT_CALC_BISHOP_ATTACKS_DIR(dir) do \
  { \
    bishop = 1 << sq; \
    while (bishop) \
    { \
      bishop = dir(bishop); \
      attacks |= bishop; \
      bishop &= ~occ; \
    } \
  } while (0)

  LUT_CALC_BISHOP_ATTACKS_DIR(noea);
  LUT_CALC_BISHOP_ATTACKS_DIR(soea);
  LUT_CALC_BISHOP_ATTACKS_DIR(nowe);
  LUT_CALC_BISHOP_ATTACKS_DIR(sowe);

#undef LUT_CALC_BISHOP_ATTACKS_DIR

  return attacks;
}

unsigned
lut_fill_bishop_attacks(bitboard mask[NUM_SQUARES], unsigned offset[NUM_SQUARES], unsigned initial_offset, bitboard lut[LUT_BISHOP_SIZE])
{
  square sq;
  unsigned current_offset, num_entries;
  bitboard board;

  current_offset = initial_offset;

  for (sq = a1; sq < NUM_SQUARES; ++sq)
  {
    offset[sq] = current_offset;

    // generate mask
    mask[sq] = 0;
    for (board = 1 << sq; board; board = noea(board), mask[sq] |= board);
    for (board = 1 << sq; board; board = soea(board), mask[sq] |= board);
    for (board = 1 << sq; board; board = nowe(board), mask[sq] |= board);
    for (board = 1 << sq; board; board = sowe(board), mask[sq] |= board);

    // GCC
    num_entries = __builtin_popcount(mask[sq]);

    for (board = 0; board < (1u << num_entries); ++board)
    {
      // X86
      lut[current_offset++] = lut_calc_bishop_attacks(_pdep_u64(board, mask[sq]), sq);
    }
  }

  return current_offset;
}


/* KING LOOK UP TABLE */
void
lut_fill_king_attacks(bitboard lut[NUM_SQUARES])
{
  square sq;
  bitboard king;

  for (sq = a1; sq < NUM_SQUARES; ++sq)
  {
    king = (1 << sq);
    lut[sq] = 0;
    lut[sq] |= no(king);
    lut[sq] |= so(king);
    lut[sq] |= ea(king);
    lut[sq] |= we(king);
    lut[sq] |= noea(king);
    lut[sq] |= soea(king);
    lut[sq] |= nowe(king);
    lut[sq] |= sowe(king);
  }
}



void
lut_gen_knight(bitboard lut[NUM_SQUARES]) { lut_fill_knight_attacks(lut); }
void
lut_gen_king(bitboard lut[NUM_SQUARES]) { lut_fill_king_attacks(lut); }
void
lut_gen_bishop_rook(bitboard lut[LUT_BISHOP_SIZE + LUT_ROOK_SIZE], bitboard bishop_mask[NUM_SQUARES], bitboard rook_mask[NUM_SQUARES], unsigned bishop_offset[NUM_SQUARES], unsigned rook_offset[NUM_SQUARES])
{
  unsigned offset = lut_fill_bishop_attacks(bishop_mask, bishop_offset, 0, lut);
  lut_fill_rook_attacks(rook_mask, rook_offset, offset, lut);
}

#ifdef LUT_GEN_EXEC
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

int
main(int argc, char **argv)
{
  if (argc != 8)
  {
    fprintf(stderr, "Required arguments: <knightLUT_file> <kingLUT_file> <bishop_rookLUT_file> <bishop_mask_file> <rook_mask_file> <bishop_offset_file> <rook_offset_file>\n");
    exit(EXIT_FAILURE);
  }

  // LINUX
#define LUT_GEN_MAP_FILE(varname, arg, LUT_size, datatype) \
  int fd_## varname = open((arg), O_CREAT | O_RDWR, 0666); \
  if (fd_## varname == -1) \
  { \
    fprintf(stderr, "Error reading %s: %s\n", (arg), strerror(errno)); \
    exit(EXIT_FAILURE); \
  } \
  if (ftruncate(fd_## varname, (LUT_size) * sizeof(datatype))) \
  { \
    fprintf(stderr, "Error truncating size of %s: %s\n", (arg), strerror(errno)); \
    exit(EXIT_FAILURE); \
  } \
  datatype *varname = mmap(0, (LUT_size) * sizeof(datatype), PROT_WRITE, MAP_SHARED, fd_## varname, 0); \
  if (varname == MAP_FAILED) \
  { \
    fprintf(stderr, "Error mapping %s: %s\n", (arg), strerror(errno)); \
    exit(EXIT_FAILURE); \
  }

  LUT_GEN_MAP_FILE(knight,        argv[1], NUM_SQUARES, bitboard);
  LUT_GEN_MAP_FILE(king,          argv[2], NUM_SQUARES, bitboard);
  LUT_GEN_MAP_FILE(bishop_rook,   argv[3], LUT_BISHOP_SIZE + LUT_ROOK_SIZE, bitboard);
  LUT_GEN_MAP_FILE(bishop_mask,   argv[4], NUM_SQUARES, bitboard);
  LUT_GEN_MAP_FILE(rook_mask,     argv[5], NUM_SQUARES, bitboard);
  LUT_GEN_MAP_FILE(bishop_offset, argv[6], NUM_SQUARES, unsigned);
  LUT_GEN_MAP_FILE(rook_offset,   argv[7], NUM_SQUARES, unsigned);

#undef LUT_GEN_MAP_FILE



  lut_gen_knight(knight);
  lut_gen_king(king);
  lut_gen_bishop_rook(bishop_rook, bishop_mask, rook_mask, bishop_offset, rook_offset);



  // LINUX
#define LUT_GEN_UNMAP_FILE(varname, arg, LUT_size, datatype) \
  if (munmap(varname, (LUT_size) * sizeof(datatype))) \
  { \
    fprintf(stderr, "Error unmapping %s: %s\n", (arg), strerror(errno)); \
    exit(EXIT_FAILURE); \
  } \
  if (close(fd_## varname)) \
  { \
    fprintf(stderr, "Error closing %s: %s\n", (arg), strerror(errno)); \
    exit(EXIT_FAILURE); \
  }

  LUT_GEN_UNMAP_FILE(knight,        argv[1], NUM_SQUARES, bitboard);
  LUT_GEN_UNMAP_FILE(king,          argv[2], NUM_SQUARES, bitboard);
  LUT_GEN_UNMAP_FILE(bishop_rook,   argv[3], LUT_BISHOP_SIZE + LUT_ROOK_SIZE, bitboard);
  LUT_GEN_UNMAP_FILE(bishop_mask,   argv[4], NUM_SQUARES, bitboard);
  LUT_GEN_UNMAP_FILE(rook_mask,     argv[5], NUM_SQUARES, bitboard);
  LUT_GEN_UNMAP_FILE(bishop_offset, argv[6], NUM_SQUARES, unsigned);
  LUT_GEN_UNMAP_FILE(rook_offset,   argv[7], NUM_SQUARES, unsigned);

#undef LUT_GEN_UNMAP_FILE



  return EXIT_SUCCESS;
}
#endif // LUT_GEN_EXEC
