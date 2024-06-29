#ifndef SCHESS_LUT_H
#define SCHESS_LUT_H

#include <schess/types.h>
#include <stddef.h>

#define LUT_BISHOP_SIZE 5248
#define LUT_ROOK_SIZE 102400

void lut_gen_knight(bitboard lut[NUM_SQUARES]);
void lut_gen_king  (bitboard lut[NUM_SQUARES]);

void lut_gen_bishop_rook(bitboard lut[LUT_BISHOP_SIZE + LUT_ROOK_SIZE],
    bitboard bishop_mask[NUM_SQUARES], bitboard rook_mask[NUM_SQUARES],
    size_t bishop_offset[NUM_SQUARES], size_t rook_offset[NUM_SQUARES]);

#endif // SCHESS_LUT_H
