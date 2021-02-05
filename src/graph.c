#include "graph.h"

#include <dos.h>
#include <conio.h>

#include <stdlib.h>
#include <string.h>

#define COLOR(p) (p | p << 2 | p << 4 | p << 6)


static const u32 CGA_EVEN = 0xB8000000L;
static const u32 CGA_ODD = 0xB8002000L;
// For rendering
static u32 ADDR[2];

static bool clippingEnabled = false;
static i16 clipArea[4];


static void set_video_mode(u16 mode) {

    union REGS in, out;
    in.h.ah = 0;
    in.h.al = mode;
    int86(0x10, &in, &out);
}


static bool clip_rect_region(
    i16* sx, i16* sy, i16* sw, i16* sh, 
    i16* dx, i16* dy) {

    i16 ow, oh;

    // Left
    ow = *sw;
    if(*dx < clipArea[0]) {

        *sw -= clipArea[0] - (*dx);
        *sx += ow-*sw;
        *dx = clipArea[0];
    }
    
    // Right
    if(*dx+*sw >= clipArea[0] + clipArea[2]) {

         *sw -= (*dx+*sw) - (clipArea[0] + clipArea[2]); 
    }

    // Top
    oh = *sh;
    if(*dy < clipArea[1]) {

        *sh -= clipArea[1] - (*dy);
        *sy += oh-*sh;
        *dy = clipArea[1];
    }
    
    // Bottom
    if(*dy+*sh >= clipArea[1] + clipArea[3]) {

        *sh -= (*dy+*sh) - (clipArea[1] + clipArea[3]);
    }

    return *sw > 0 && *sh > 0;
}



static void draw_text_base(
    void (*draw_func) (Bitmap*, i16, i16, i16, i16, i16, i16),
    Bitmap* font, const str text, 
    i16 x, i16 y, i16 endIndex, bool center) {

    i16 dx, dy;
    i16 sx, sy;
    i16 i = 0;
    i16 d = font->width / 16;
    char c;

    if (endIndex < 0)
        endIndex = strlen(text);

    if (center) {

        x -= strlen(text) * (d / 4) / 2;
    }
    dx = x;
    dy = y;

    while (i < endIndex && (c = text[i ++]) != '\0') {

        if (c == '\n') {

            dx = x;
            dy += d;
            continue;
        }

        sx = ((i16)c) % 16;
        sy = ((i16)c) / 16;

        draw_func(font, 
            sx*(d / 4), sy*d, 
            d / 4, d,
            dx, dy);
        

        dx += d / 4;
    }
}


void init_graphics() {

    const u16 CGA_MODE = 4;

    set_video_mode(CGA_MODE);

    ADDR[0] = CGA_EVEN;
    ADDR[1] = CGA_ODD;

    clippingEnabled = false;
    clipArea[0] = 0;
    clipArea[1] = 0;
    clipArea[2] = 80;
    clipArea[3] = 200;
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
    u32 jump;
    u8 p = COLOR(color);

    jump = (u32)((y/2)*80 + x);
    for (i = y; i < y+h; ++ i) {

        memset((void*)(ADDR[i & 1] + jump), p, w);
        jump += 80 * (i & 1);
    }
}


void vertical_line(i16 x, i16 y, u8 shift, i16 h, u8 color) {

    static const u8 MASK[] = {
        3, 12, 48, 192};

    u8 mask = MASK[shift % 4];
    i16 dy;
    u32 djump;
    u8* out;

    djump = (u32)((y/2) * 80 + x);
    for (dy = y; dy < y + h; ++ dy) {

        out = (u8*)ADDR[dy & 1];
        out[djump] = (color & mask) | 
                (out[djump] & ~mask);

        djump += 80 * (dy & 1);
    }
}


void draw_bitmap_fast(Bitmap* bmp, i16 x, i16 y) {

    draw_bitmap_region_fast(bmp, 0, 0, bmp->width/4, bmp->height, x, y);
}


void draw_bitmap_region_fast(Bitmap* bmp, 
    i16 sx, i16 sy, i16 sw, i16 sh, 
    i16 dx, i16 dy) {

    i16 i;
    u32 djump;
    u32 sjump;
    u16 w = bmp->width / 4;

    djump = (u32)((dy/2)*80 + dx);
    sjump = (u32)(sy*w + sx);

    if (clippingEnabled && !clip_rect_region(&sx, &sy, &sw, &sh, &dx, &dy))
        return;

    for (i = dy; i < dy + sh; ++ i) {

        memcpy((void*)(ADDR[i & 1] + djump), 
               (void*)((u32)bmp->pixels + sjump), sw);

        djump += 80 * (i & 1);
        sjump += w;
    }     
}


// For faster performance ordinary fast_region does not
// call this method with "big enough" skip value
void draw_bitmap_region_fast_skip_lines(Bitmap* bmp, 
    i16 sx, i16 sy, i16 sw, i16 sh, 
    i16 dx, i16 dy, i16 skip) {
    
    i16 i;
    u32 djump;
    u32 sjump;
    u16 w = bmp->width / 4;

    djump = (u32)((dy/2)*80 + dx);
    sjump = (u32)(sy*w + sx);

    if (skip == 0 || (
        clippingEnabled && 
        !clip_rect_region(&sx, &sy, &sw, &sh, &dx, &dy)))
        return;

    for (i = dy; i < dy + sh; ++ i) {

        // Notice "== 1" on the bottom line.
        // It is required to maintain a nice effect. (What)
        if ((skip > 0 && i % skip != 0) ||
            (skip < 0 && i % (-skip) == 1)) {

            memcpy((void*)(ADDR[i & 1] + djump), 
                (void*)((u32)bmp->pixels + sjump), sw);
        }

        djump += 80 * (i & 1);
        sjump += w;
    }     
}


void draw_text_fast(Bitmap* font, const str text, 
    i16 x, i16 y, i16 endIndex, bool center) {

    draw_text_base(draw_bitmap_region_fast,
        font, text, x, y, endIndex, center);
}


void draw_bitmap_region(Bitmap* bmp, 
    i16 sx, i16 sy, i16 sw, i16 sh, 
    i16 dx, i16 dy) {

    i16 x, y;
    u32 djump;
    u32 sjump;
    u16 w = bmp->width / 4;
    u8 mask;

    u8* out;

    if (clippingEnabled && !clip_rect_region(&sx, &sy, &sw, &sh, &dx, &dy))
        return;

    if (bmp->mask == NULL) {

        draw_bitmap_region_fast(bmp, sx, sy, sw, sh, dx, dy);
        return;
    }

    djump = (u32)((dy/2)*80 + dx);
    sjump = (u32)(sy*w + sx);

    for (y = dy; y < dy + sh; ++ y) {   

        out = (u8*)ADDR[y & 1];
        for (x = 0; x < sw; ++ x) {

            mask = bmp->mask[sjump];
            out[djump] = ((bmp->pixels[sjump]) & mask) | 
                            (out[djump] & ~mask);

            ++ sjump;
            ++ djump;
        }
        djump += 80 * (y & 1) - sw;
        sjump += w - sw;
    }   
}


void draw_text(Bitmap* font, const str text, i16 x, i16 y, i16 endIndex, bool center) {

    draw_text_base(draw_bitmap_region,
        font, text, x, y, endIndex, center);
}


void draw_sprite(Sprite* spr, Bitmap* bmp, i16 x, i16 y) {

    draw_bitmap_region(bmp, 
        spr->frame * spr->width, spr->row * spr->height,
        spr->width, spr->height, x, y);
}


void vblank() {

    while (inp(0x3DA) & 8);
    while (!(inp(0x3DA) & 8));
}


void toggle_clipping(bool state) {

    clippingEnabled = state;
}


void set_clip_rectangle(i16 x, i16 y, i16 w, i16 h) {

    clipArea[0] = x;
    clipArea[1] = y;
    clipArea[2] = w;
    clipArea[3] = h;
}
