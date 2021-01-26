#ifndef __GAME__
#define __GAME__

#include "types.h"

typedef void* GameState;

bool game_refresh(void* self, i16 step);
void game_redraw(void* self);

GameState new_game_state();
void dispose_game_state(GameState state);

#endif // __GAME__
