#include "stage.h"

#include "graph.h"
#include "util.h"
#include "err.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static i16 get_tile(Stage* s, i16 x, i16 y, i16 def) {

    if (x < 0 || y < 0 || x >= s->roomWidth || y >= s->roomHeight)
        return def;

    return s->roomTiles[y * s->roomWidth + x];
}


static TileWallData gen_tile_wall_data(Stage* s, i16 dx, i16 dy) {

    TileWallData t;

    i16 i, x, y;
    bool neighbourhood[9];

    for (i = 0; i < 4; ++ i) {

        t.srcx[i] = (i % 2);
        t.srcy[i] = (i / 2);
    }

    for (x = 0; x < 3; ++ x) {

        for (y = 0; y < 3; ++ y) {

            neighbourhood[y * 3 + x] = get_tile(s, dx+x-1, dy+y-1, 1) == 1;
        }
    }

    // Bottom
    if (!neighbourhood[7]) {

        if (neighbourhood[3]) 
            t.srcx[2] = 2;
        if (neighbourhood[5]) 
            t.srcx[3] = 2;
    }

    // Top
    if (!neighbourhood[1]) {

        if (neighbourhood[3]) 
            t.srcx[0] = 2;
        if (neighbourhood[5]) 
            t.srcx[1] = 2;
    }

    // Right
    if (!neighbourhood[5]) {

        t.srcy[1] = 1;
        t.srcy[3] = 1;

        t.srcx[1] = 3;
        t.srcx[3] = 3;

        if (!neighbourhood[1]) {

            t.srcy[1] = 0;
            t.srcx[1] = 1;
        }

        if (!neighbourhood[7]) {

            t.srcy[3] = 1;
            t.srcx[3] = 1;
        }
    }

    // Left
    if (!neighbourhood[3]) {

        t.srcy[0] = 0;
        t.srcy[2] = 0;

        t.srcx[0] = 3;
        t.srcx[2] = 3;

        if (!neighbourhood[1]) {

            t.srcy[0] = 0;
            t.srcx[0] = 0;
        }

        if (!neighbourhood[7]) {

            t.srcy[2] = 1;
            t.srcx[2] = 0;
        }
    }

    // "Empty" corners
    if (neighbourhood[1] && neighbourhood[3]) {

        t.srcx[0] = 4 + 2 * (i16)neighbourhood[0];
    }
    if (neighbourhood[1] && (i16)neighbourhood[5]) {

        t.srcx[1] = 5 + 2 * (i16)neighbourhood[2];
    }
    if (neighbourhood[7] && (i16)neighbourhood[3]) {

        t.srcx[2] = 4 + 2 * (i16)neighbourhood[6];
    }
    if (neighbourhood[7] && neighbourhood[5]) {

        t.srcx[3] = 5 + 2 * (i16)neighbourhood[8];
    }

    return t;
}


static void gen_wall_tile_map(Stage* s) {
    
    i16 x, y;

    for (y = 0; y < s->roomHeight; ++ y) {

        for (x = 0; x < s->roomWidth; ++ x) {

            switch (get_tile(s, x, y, 1)) {

            case 1:
                s->wallTiles[y * s->roomWidth + x] = gen_tile_wall_data(s, x, y);
                break;
            
            default:
                break;
            }
        }
    }
}


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

    s->roomTiles = (i16*)malloc(sizeof(i16) * roomWidth * roomHeight);
    if (s->roomTiles == NULL) {

        dispose_stage(s);
        return NULL;
    }
    tmap_clone_area_i16(baseMap, s->roomTiles,
        0, camX, camY, roomWidth, roomHeight);

    s->wallTiles = (TileWallData*) malloc(sizeof(TileWallData) * roomWidth * roomHeight);
    if (s->wallTiles == NULL) {

        dispose_stage(s);
        return NULL;
    }
    gen_wall_tile_map(s);

    s->yoff = 100 - roomHeight*8;
    s->xoff = 30 - roomWidth*2;

    return s;
}


void dispose_stage(Stage* s) {

    if (s == NULL) return;

    if (s->renderBuffer != NULL)
        free(s->renderBuffer);
    if (s->wallTiles != NULL)
        free(s->wallTiles);
    if (s->roomTiles != NULL)
        free(s->roomTiles);

    free(s);
}


static void stage_draw_wall(Stage* s, Bitmap* bmpTileset,
    i16 dx, i16 dy) {

    TileWallData* t = &s->wallTiles[dy * s->roomWidth + dx];
    i16 x, y, i;

    for (y = 0; y < 2; ++ y) {

        for (x = 0; x < 2; ++ x) {

            i = y * 2 + x;

            draw_bitmap_region_fast(bmpTileset,
                t->srcx[i] * 2, t->srcy[i] * 8, 2, 8, 
                s->xoff + dx*4 + x*2,
                s->yoff + dy*16 + y*8);
        }
    }
}


void stage_draw(Stage* s, Bitmap* bmpTileset) {

    i16 x, y;
    i16 index;
    i16 tid;

    for (y = 0; y < s->roomHeight; ++ y) {

        for (x = 0; x < s->roomWidth; ++ x) {

            index = y * s->roomWidth + x;
            if (!s->renderBuffer[index])
                continue;
            s->renderBuffer[index] = false;

            tid = get_tile(s, x, y, -1);
            if (tid < 0) continue;

            switch (tid) {

            case 0:

                draw_bitmap_region_fast(bmpTileset,
                    16, 0, 4, 16, 
                    s->xoff + x*4,
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


void stage_mark_tile_for_redraw(Stage* s, i16 x, i16 y) {

    if (x < 0 || y < 0 || x >= s->roomWidth || y >= s->roomHeight)
        return;

    s->renderBuffer[y * s->roomWidth + x] = true;
}


bool stage_is_tile_solid(Stage* s, i16 x, i16 y) {

    if (x < 0 || y < 0 || 
        x >= s->roomWidth || 
        y >= s->roomHeight)
        return true;

    return get_tile(s, x, y, 1) == 1;
}


bool stage_check_camera_transition(Stage* s, i16 x, i16 y) {

    const i16 JUMP_X[4] = {1, 0, -1, 0};
    const i16 JUMP_Y[4] = {0, -1, 0, 1};

    i16 dir = 0;
    if (x >= s->roomWidth) dir = 1;
    if (y < 0) dir = 2;
    if (x < 0) dir = 3;
    if (y >= s->roomHeight) dir = 4;

    if (dir != 0) {

        s->camPos.x += JUMP_X[dir-1] * s->roomWidth;
        s->camPos.y += JUMP_Y[dir-1] * s->roomHeight;

        tmap_clone_area_i16(s->baseMap, 
            s->roomTiles, 0, 
            s->camPos.x, s->camPos.y, 
            s->roomWidth, s->roomHeight);
        gen_wall_tile_map(s);

        memset(s->renderBuffer, 1, s->roomWidth * s->roomHeight);

        return true;
    }
    return false;
}
