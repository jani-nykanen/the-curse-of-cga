#include "graph.h"

#include <dos.h>
#include <conio.h>

#include <stdlib.h>
#include <string.h>


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

    u8 p = color | (color << 2) | (color << 4) | (color << 6);

    memset((void*)CGA_EVEN, p, 8000);
    memset((void*)CGA_ODD, p, 8000);
}


void vblank() {

    while (inp(0x3DA) & 8);
    while (!(inp(0x3DA) & 8));
}
