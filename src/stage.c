#include "stage.h"

#include "graph.h"
#include "util.h"
#include "err.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const i16 X_OFF = 4;


Stage* new_stage(Tilemap* baseMap, 
    i16 roomWidth, i16 roomHeight, i16 camX, i16 camY) {

    Stage* alloc_object(s, Stage, NULL);

    s->renderBuffer = (u8*) malloc(sizeof(bool) * roomWidth * roomHeight);
    if (s->renderBuffer == NULL) {

        free(s);
        return NULL;
    }

    memset(s->renderBuffer, 1, roomWidth * roomHeight);

    s->baseMap = baseMap;
    s->roomWidth = roomWidth;
    s->roomHeight = roomHeight;
    s->camPos = vec2(camX, camY);

    s->yoff = 100 - roomHeight*8;

    return s;
}


void dispose_stage(Stage* s) {

    if (s == NULL) return;

    free(s->renderBuffer);
    free(s);
}


static void stage_draw_wall(Stage* s, Bitmap* bmpTileset,
    i16 dx, i16 dy) {

    i16 i, x, y;
    bool neighbourhood[9];
    i16 srcx[4];
    i16 srcy[4];

    for (i = 0; i < 4; ++ i) {

        srcx[i] = (i % 2);
        srcy[i] = (i / 2);
    }

    for (x = 0; x < 3; ++ x) {

        for (y = 0; y < 3; ++ y) {

            neighbourhood[y * 3 + x] = tmap_get_tile(s->baseMap, 0, dx+x-1, dy+y-1) == 1;
        }
    }

    // Bottom
    if (!neighbourhood[7]) {

        if (neighbourhood[3]) 
            srcx[2] = 2;
        if (neighbourhood[5]) 
            srcx[3] = 2;
    }

    // Top
    if (!neighbourhood[1]) {

        if (neighbourhood[3]) 
            srcx[0] = 2;
        if (neighbourhood[5]) 
            srcx[1] = 2;
    }

    // Right
    if (!neighbourhood[5]) {

        srcy[1] = 1;
        srcy[3] = 1;

        srcx[1] = 3;
        srcx[3] = 3;

        if (!neighbourhood[1]) {

            srcy[1] = 0;
            srcx[1] = 1;
        }

        if (!neighbourhood[7]) {

            srcy[3] = 1;
            srcx[3] = 1;
        }
    }

    // Left
    if (!neighbourhood[5]) {

        srcy[0] = 0;
        srcy[2] = 0;

        srcx[0] = 3;
        srcx[2] = 3;

        if (!neighbourhood[1]) {

            srcy[0] = 0;
            srcx[0] = 0;
        }

        if (!neighbourhood[7]) {

            srcy[2] = 1;
            srcx[2] = 0;
        }
    }


    for (y = 0; y < 2; ++ y) {

        for (x = 0; x < 2; ++ x) {

            i = y * 2 + x;

            draw_bitmap_region_fast(bmpTileset,
                srcx[i] * 2, srcy[i] * 8, 2, 8, 
                X_OFF + dx*4 + x*2,
                s->yoff + dy*16 + y*8);
        }
    }
}


void stage_draw(Stage* s, Bitmap* bmpTileset) {

    i16 x, y;
    i16 index;
    i16 tid;

    for (y = 0; y < s->baseMap->height; ++ y) {

        for (x = 0; x < s->baseMap->width; ++ x) {

            index = y * s->roomWidth + x;
            if (!s->renderBuffer[index])
                continue;
            s->renderBuffer[index] = false;

            tid = tmap_get_tile(s->baseMap, 0, x, y);
            if (tid < 0) continue;

            switch (tid) {

            case 0:

                draw_bitmap_region_fast(bmpTileset,
                    12, 0, 4, 16, 
                    X_OFF + x*4,
                    s->yoff + y*16);
                break;

            case 1:

                stage_draw_wall(s, bmpTileset, x, y);
                break;
            
            default:
                break;
            }
        }
    }
}
