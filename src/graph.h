#ifndef __GRAPHICS__
#define __GRAPHICS__

#include "types.h"

void init_graphics();
void reset_graphics();

void clear_screen(u8 color);

//
// Note on drawing functions:
// x coordinates are multiplied by 4 for faster
// rendering, and clipping is not implemented 
// since it's not needed in this project
//

void fill_rect(i16 x, i16 y, i16 w, i16 h, u8 color);

void vblank();

#endif // __GRAPHICS__
