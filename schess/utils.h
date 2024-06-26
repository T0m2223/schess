#ifndef SCHESS_UTILS_H
#define SCHESS_UTILS_H

#include <schess/types.h>

int
parse_FEN(const char *FEN, board_state *board, irreversable_state *meta);

void
print_board(board_state *board);

#endif // SCHESS_UTILS_H
