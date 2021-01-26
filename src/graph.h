#ifndef __GRAPHICS__
#define __GRAPHICS__

#include "types.h"

void init_graphics();
void reset_graphics();

void clear_screen(u8 color);

void vblank();

#endif // __GRAPHICS__
