#ifndef __TITLE_SCREEN__
#define __TITLE_SCREEN__

#include "types.h"

bool init_title_screen_scene();
bool title_screen_refresh(i16 step);
void title_screen_redraw();

void dispose_title_screen_scene();

#endif // __TITLE_SCREEN__
