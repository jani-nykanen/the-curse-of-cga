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
    i16 x, i16 y) {

    draw_bitmap_region_fast(bmpTileset,
        0, 0, 4, 16, 
        X_OFF + x*4,
        s->yoff + y*16);
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
