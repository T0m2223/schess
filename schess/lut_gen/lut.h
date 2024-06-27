#ifndef SCHESS_LUT_H
#define SCHESS_LUT_H

#include <schess/types.h>

#define LUT_BISHOP_SIZE 2290
#define LUT_ROOK_SIZE 65536

void lut_gen_knight(bitboard lut[NUM_SQUARES]);
void lut_gen_king  (bitboard lut[NUM_SQUARES]);

void lut_gen_bishop_rook(bitboard lut[LUT_BISHOP_SIZE + LUT_ROOK_SIZE],
    bitboard bishop_mask[NUM_SQUARES], bitboard rook_mask[NUM_SQUARES],
    unsigned bishop_offset[NUM_SQUARES], unsigned rook_offset[NUM_SQUARES]);

#endif // SCHESS_LUT_H
