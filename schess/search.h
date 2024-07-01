#ifndef SCHESS_SEARCH_H
#define SCHESS_SEARCH_H

#include <schess/types.h>

move search_best_move(game_state *game, irreversable_state meta, unsigned depth);

#endif // SCHESS_SEARCH_H
