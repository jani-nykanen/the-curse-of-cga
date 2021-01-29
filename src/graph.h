#ifndef __GRAPHICS__
#define __GRAPHICS__

#include "types.h"
#include "bitmap.h"
#include "sprite.h"


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

void draw_bitmap_fast(Bitmap* bmp, i16 x, i16 y);
void draw_bitmap_region_fast(Bitmap* bmp, 
    i16 sx, i16 sy, i16 sw, i16 sh, 
    i16 dx, i16 dy);
void draw_text_fast(Bitmap* font, const str text, i16 x, i16 y, bool center);

void draw_bitmap_region(Bitmap* bmp, 
    i16 sx, i16 sy, i16 sw, i16 sh, 
    i16 dx, i16 dy);
void draw_text(Bitmap* font, const str text, i16 x, i16 y, bool center);

void draw_sprite(Sprite* spr, Bitmap* bmp, i16 x, i16 y);

void vblank();

void toggle_clipping(bool state);
void set_clip_rectangle(i16 x, i16 y, i16 w, i16 h);

#endif // __GRAPHICS__
