#include "graph.h"

#include <dos.h>
#include <conio.h>

#include <stdlib.h>
#include <string.h>

#define COLOR(p) (p | p << 2 | p << 4 | p << 6)


static const u32 CGA_EVEN = 0xB8000000L;
static const u32 CGA_ODD = 0xB8002000L;


static void set_video_mode(u16 mode) {

    union REGS in, out;
    in.h.ah = 0;
    in.h.al = mode;
    int86(0x10, &in, &out);
}



void init_graphics() {

    const u16 CGA_MODE = 4;

    set_video_mode(CGA_MODE);
}


void reset_graphics() {

    // Is 0 even the default video mode?
    set_video_mode(0);  
}


void clear_screen(u8 color) {

    u8 p = COLOR(color);

    memset((void*)CGA_EVEN, p, 8000);
    memset((void*)CGA_ODD, p, 8000);
}


void fill_rect(i16 x, i16 y, i16 w, i16 h, u8 color) {

    i32 i;
    u32 jump;
    u8 p = COLOR(color);

    for (i = 0; i < h; i += 2) {

        jump = (u32)(((y + i)/2)*80 + x);
        memset((void*)(CGA_ODD + jump), p, w);

        jump = (u32)(((y + i)/2 + 1)*80 + x);
        memset((void*)(CGA_EVEN + jump), p, w);
    }
}


void vblank() {

    while (inp(0x3DA) & 8);
    while (!(inp(0x3DA) & 8));
}
