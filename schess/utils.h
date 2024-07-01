#ifndef SCHESS_UTILS_H
#define SCHESS_UTILS_H

#include <schess/gen.h>
#include <schess/types.h>

const char *
move_name(enum MOVE_TYPE mt);
const char *
piece_name(piece_type pt);
const char *
square_name(square sq);

int parse_FEN(const char *FEN, game_state *game, irreversable_state *meta);

void print_board(board_state *board);
void print_moves(board_state *board, struct move_buffer *mbuf);
void print_move(board_state *board, move m);

#endif // SCHESS_UTILS_H
