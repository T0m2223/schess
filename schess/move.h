#ifndef SCHESS_MOVES_H
#define SCHESS_MOVES_H

#include <schess/types.h>

int
move_make(move *m, board_state *board, irreversable_state *meta);

int
move_unmake(move *m, board_state *board);

#endif // SCHESS_MOVES_H
