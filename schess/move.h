#ifndef SCHESS_MOVES_H
#define SCHESS_MOVES_H

#include <schess/types.h>

int
move_make(move *m, game_state *game, irreversable_state *meta);

int
move_unmake(move *m, game_state *game);

#endif // SCHESS_MOVES_H
