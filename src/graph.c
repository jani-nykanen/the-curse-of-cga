#include "graph.h"

#include <dos.h>
#include <conio.h>

#include <stdlib.h>
#include <string.h>

#define COLOR(p) (p | p << 2 | p << 4 | p << 6)


static const ulong CGA_EVEN = 0xB8000000L;
static const ulong CGA_ODD = 0xB8002000L;
// For rendering
static ulong ADDR[2];


static void set_video_mode(u16 mode) {

    union REGS in, out;
    in.h.ah = 0;
    in.h.al = mode;
    int86(0x10, &in, &out);
}



void init_graphics() {

    const u16 CGA_MODE = 4;

    set_video_mode(CGA_MODE);

    ADDR[0] = CGA_EVEN;
    ADDR[1] = CGA_ODD;
}


void reset_graphics() {

    const u16 POSSIBLE_DEFAULT_MODE = 2;

    // Is this even the default video mode?
    set_video_mode(POSSIBLE_DEFAULT_MODE);  
}


void clear_screen(u8 color) {

    u8 p = COLOR(color);

    memset((void*)CGA_EVEN, p, 8000);
    memset((void*)CGA_ODD, p, 8000);
}


void fill_rect(i16 x, i16 y, i16 w, i16 h, u8 color) {

    i16 i;
    ulong jump;
    u8 p = COLOR(color);

    jump = (ulong)((y/2)*80 + x);
    for (i = y; i < y+h; ++ i) {

        memset((void*)(ADDR[i & 1] + jump), p, w);
        jump += 80 * (i & 1);
    }
}


void draw_bitmap_fast(Bitmap* bmp, i16 x, i16 y) {

    draw_bitmap_region_fast(bmp, 0, 0, bmp->width/4, bmp->height, x, y);
}


void draw_bitmap_region_fast(Bitmap* bmp, i16 sx, i16 sy, i16 sw, 
    i16 sh, i16 dx, i16 dy) {

    i16 i;
    ulong djump;
    ulong sjump;
    u16 w = bmp->width / 4;

    djump = (ulong)((dy/2)*80 + dx);
    sjump = (ulong)(sy*w + sx);

    for (i = dy; i < dy + sh; ++ i) {

        memcpy((void*)(ADDR[i & 1] + djump), 
               (void*)((ulong)bmp->pixels + sjump), sw);

        djump += 80 * (i & 1);
        sjump += w;
    }     
}


void draw_text_fast(Bitmap* font, const str text, i16 x, i16 y, bool center) {

    i16 dx, dy;
    i16 sx, sy;
    i16 i = 0;
    i16 d = font->width / 16;
    char c;

    if (center) {

        x -= strlen(text) * (d / 4);
    }
    dx = x;
    dy = y;

    while ((c = text[i ++]) != '\0') {

        if (c == '\n') {

            dx = x;
            dy += d;
            continue;
        }

        sx = ((i16)c) % 16;
        sy = ((i16)c) / 16;

        draw_bitmap_region_fast(font, 
            sx*(d / 4), sy*d, 
            d / 4, d,
            dx, dy);

        dx += d / 4;
    }
}


void vblank() {

    while (inp(0x3DA) & 8);
    while (!(inp(0x3DA) & 8));
}
