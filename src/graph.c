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
    ulong jump;
    u8 p = COLOR(color);

    ulong addr1 = ADDR[y % 2];
    ulong addr2 = ADDR[(y+1) % 2];

    for (i = 0; i < h; i += 2) {

        jump = (ulong)(((y + i)/2)*80 + x);
        memset((void*)(addr1 + jump), p, w);

        jump += (ulong)((y % 2) * 80);
        memset((void*)(addr2 + jump), p, w);
    }

}


void vblank() {

    while (inp(0x3DA) & 8);
    while (!(inp(0x3DA) & 8));
}
