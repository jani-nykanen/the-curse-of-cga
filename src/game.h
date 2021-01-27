#ifndef __GAME__
#define __GAME__


#include "types.h"

bool init_game_scene();
bool game_refresh(i16 step);
void game_redraw();

void dispose_game_scene();

#endif // __GAME__
