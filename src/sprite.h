#ifndef __SPRITE__
#define __SPRITE__

#include "types.h"

typedef struct {

    i16 width;
    i16 height; 
    i16 frame;
    i16 row;
    i16 count;

} Sprite;

Sprite create_sprite(i16 w, i16 h);

void spr_animate(Sprite* s, 
    i16 row, i16 start, i16 end, 
    i16 speed, i16 steps);

void spr_set_frame(Sprite* s, i16 frame, i16 row);

#endif // __SPRITE__
