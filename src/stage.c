#include "stage.h"

#include "graph.h"
#include "util.h"
#include "err.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static bool is_solid(u8 v) {

    static const bool TABLE[] = {
        0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1,

        // "Overflow" values, remove in the
        // release version
        0, 0, 0, 0, 0, 0, 0, 0
    };

    return TABLE[(i16)v];
}


static bool is_solid_ignore_water(u8 v) {

    if (v == 3)
        return false;

    return is_solid(v);
}


static bool is_item(u8 v) {

    return v >= 17;
}


static u8 get_tile(Stage* s, u8* arr, i16 x, i16 y, u8 def) {

    if (x < 0 || y < 0 || x >= s->roomWidth || y >= s->roomHeight)
        return def;

    return arr[y * s->roomWidth + x];
}


static u8 get_tile_either(Stage* s, i16 x, i16 y, u8 def) {

    u8 id = get_tile(s, s->roomTilesStatic, x, y, def);
    if (id == 0)
        return get_tile(s, s->roomTilesDynamic, x, y, def);

    return id;
}


static void set_tile(Stage* s, u8* arr, i16 x, i16 y, u8 v) {

    i16 i;

    if (x < 0 || y < 0 || x >= s->roomWidth || y >= s->roomHeight)
        return;

    i = y * s->roomWidth + x;

    s->renderBuffer[i] = true;
    arr[i] = v;
}


static void set_tile_both(Stage* s, i16 x, i16 y, u8 v) {

    set_tile(s, s->roomTilesStatic, x, y, v);
    set_tile(s, s->roomTilesDynamic, x, y, v);
}


static void set_tile_permanent(Stage* s, i16 x, i16 y, u8 v) {

    set_tile_both(s, x, y, v);
    tmap_set_tile(s->baseMap, 0, x, y, v);
}


static void check_confict(Stage* s, i16 x, i16 y) {

    // Rock & water
    if (get_tile(s, s->roomTilesDynamic, x, y, 0) == 2 &&
        get_tile(s, s->roomTilesStatic, x, y, 0) == 3) {

        set_tile_both(s, x, y, 0);
    }
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

            neighbourhood[y * 3 + x] = get_tile(s, s->roomTilesStatic, 
                dx+x-1, dy+y-1, 1) == 1;
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

            switch (get_tile(s, s->roomTilesStatic, x, y, 1)) {

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

    // Static tiles for objects that cannot be moved,
    // and that can be overlaid
    s->roomTilesStatic = (u8*)malloc(roomWidth * roomHeight);
    if (s->roomTilesStatic == NULL) {

        dispose_stage(s);
        return NULL;
    }
    tmap_clone_area(baseMap, s->roomTilesStatic,
        0, camX, camY, roomWidth, roomHeight);

    // Dynamic objects may overlay static ones
    s->roomTilesDynamic = (u8*)malloc(roomWidth * roomHeight);
    if (s->roomTilesDynamic == NULL) {

        dispose_stage(s);
        return NULL;
    }
    memcpy(s->roomTilesDynamic, s->roomTilesStatic, 
        roomWidth * roomHeight);

    s->wallTiles = (TileWallData*) malloc(sizeof(TileWallData) * roomWidth * roomHeight);
    if (s->wallTiles == NULL) {

        ERR_MALLOC();
        dispose_stage(s);
        return NULL;
    }
    gen_wall_tile_map(s);

    s->yoff = 100 - roomHeight*8;
    s->xoff = 30 - roomWidth*2;

    s->rockAnim.timer = 0;

    return s;
}


void dispose_stage(Stage* s) {

    if (s == NULL) return;

    if (s->renderBuffer != NULL)
        free(s->renderBuffer);
    if (s->wallTiles != NULL)
        free(s->wallTiles);
    if (s->roomTilesStatic != NULL)
        free(s->roomTilesStatic);
    if (s->roomTilesDynamic != NULL)
        free(s->roomTilesDynamic);    

    free(s);
}


void stage_update(Stage* s, i16 step) {

    i16 moveStep;
    i16 delta;

    if (s->rockAnim.timer > 0) {

        stage_mark_tile_for_redraw(s, 
                s->rockAnim.pos.x, 
                s->rockAnim.pos.y);
        stage_mark_tile_for_redraw(s, 
                s->rockAnim.target.x, 
                s->rockAnim.target.y);

        s->rockAnim.timer -= step;
        if (s->rockAnim.timer <= 0) {

            check_confict(s, s->rockAnim.target.x, s->rockAnim.target.y);
            return;
        }

        moveStep = s->rockAnim.startTime / 4;
        delta = 4 - fixed_round(s->rockAnim.timer, moveStep);

        s->rockAnim.rpos.x = s->rockAnim.pos.x * 4 + 
            (s->rockAnim.target.x - s->rockAnim.pos.x) * delta;
        s->rockAnim.rpos.y = s->rockAnim.pos.y * 16 + 
            (s->rockAnim.target.y - s->rockAnim.pos.y) * delta * 4;
    }
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
    i16 dx, dy;
    i16 sx, sy;
    i16 index;
    u8 tid;

    // TODO: Look-up table? Most of the tiles
    // are normal
    for (y = 0; y < s->roomHeight; ++ y) {

        for (x = 0; x < s->roomWidth; ++ x) {

            index = y * s->roomWidth + x;
            if (!s->renderBuffer[index])
                continue;
            // s->renderBuffer[index] = false;

            tid = get_tile(s, s->roomTilesStatic, x, y, 0);
            dx = s->xoff + x*4;
            dy = s->yoff + y*16;

            switch (tid) {

            // Wall
            case 1:
                stage_draw_wall(s, bmpTileset, x, y);
               continue;

            // Water
            case 3:

                sx = 16;
                sy = 16;
                break;
            
            // Special walls
            case 4:
            case 5:

                sx = (tid-4) * 4;
                sy = 16;
                break;

            // Bolt
            case 6:
            case 7:

                sx = 8 + 4 * (tid-6);
                sy = 16;
                break;

            // Flower
            case 8:

                sx = 20;
                sy = 0;
                break;

            // Ice block
            case 9:

                sx = 20;
                sy = 16;
                break;

            // Key hole
            case 10:

                sx = 24;
                sy = 0;
                break;

            default:
                sx = 16;
                sy = 0;
                break;
            }

            if (tid >= 17) {

                sx = 4 * (tid - 17);
                sy = 32;
            }

            draw_bitmap_region_fast(bmpTileset,
                sx, sy, 4, 16, 
                dx, dy);
        }
    }
}


// We draw objects in a separate loop since
// they might - and do - overlay static tiles
void stage_draw_objects(Stage* s, Bitmap* bmpObjects) {

    i16 x, y;
    i16 dx, dy;
    i16 index;
    u8 tid;

    for (y = 0; y < s->roomHeight; ++ y) {

        for (x = 0; x < s->roomWidth; ++ x) {

            index = y * s->roomWidth + x;
            if (!s->renderBuffer[index])
                continue;
            s->renderBuffer[index] = false;

            tid = get_tile(s, s->roomTilesDynamic, x, y, 0);
            if (tid == 0) continue;

            dx = s->xoff + x*4;
            dy = s->yoff + y*16;

            switch (tid) {

            case 2:

                if (s->rockAnim.timer <= 0 ||
                    s->rockAnim.target.x != x ||
                    s->rockAnim.target.y != y) {

                    draw_bitmap_region(bmpObjects,
                        0, 0, 4, 16, 
                        dx, dy);
                }

                break;
            
            default:
                break;
            }
        }
    }

    if (s->rockAnim.timer > 0) {

        draw_bitmap_region(bmpObjects,
            0, 0, 4, 16, 
            s->xoff + s->rockAnim.rpos.x,
            s->yoff + s->rockAnim.rpos.y);
    }
}


void stage_mark_tile_for_redraw(Stage* s, i16 x, i16 y) {

    if (x < 0 || y < 0 || x >= s->roomWidth || y >= s->roomHeight)
        return;

    s->renderBuffer[y * s->roomWidth + x] = true;
}


static void swap_walls(Stage* s, i16 dx, i16 dy) {

    i16 x, y;
    u8 tid;

    // Swap walls
    for (y = 0; y < s->roomHeight; ++ y) {

        for (x = 0; x < s->roomWidth; ++ x) {

            tid = get_tile(s, s->roomTilesStatic, x, y, 0);
            if (tid == 4 || tid == 5) {

                set_tile_both(s, x, y, tid == 4 ? 5 : 4);
            }

            tid = get_tile(s, s->roomTilesStatic, x, y, 0);
            if (tid == 7) {

                set_tile_both(s, x, y, 6);
            }

        }
    }

    set_tile_both(s, dx, dy, 7);
}


static void cut_flower(Stage* s, i16 x, i16 y) {

    set_tile_both(s, x, y, 0);
}


static void break_ice_block(Stage* s, i16 x, i16 y) {

    set_tile(s, s->roomTilesStatic, x, y, 0);
    set_tile(s, s->roomTilesDynamic, x, y, 2);
}


static void open_lock(Stage* s, i16 x, i16 y) {

    set_tile_permanent(s, x, y, 0);
}


u8 stage_movement_collision(Stage* s, 
    State actionType, i16 x, i16 y, 
    i16 dx, i16 dy, i16 objectMoveTime,
    u8* interactionLevel, u8* keyCount) {

    u8 id = get_tile(s, s->roomTilesDynamic, x, y, 0);
    u8 mid;

    switch (id) {

    // Rock
    case 2:

        mid = get_tile(s, s->roomTilesDynamic, x+dx, y+dy, 1);
        if (!is_solid_ignore_water(mid)) {

            set_tile(s, s->roomTilesDynamic, x, y, 0);
            set_tile(s, s->roomTilesDynamic, x+dx, y+dy, 2);

            s->rockAnim.pos = vec2(x, y);
            s->rockAnim.target = vec2(x+dx, y+dy);
            s->rockAnim.startTime = objectMoveTime;
            s->rockAnim.timer = objectMoveTime;

            return 1;
        }
        return 0;

    // Bolt
    case 6:

        if (actionType == STATE_PRESSED) {

            swap_walls(s, x, y);
            return 2;
        }
        return 0;

    // Flower
    case 8:

        if ((*interactionLevel) > 0 && 
            actionType == STATE_PRESSED) {

            cut_flower(s, x, y);
            -- (*interactionLevel);

            return 2;
        }
        return 0;

    // Ice block
    case 9:

        if ((*interactionLevel) > 0 &&
            actionType == STATE_PRESSED) {

            break_ice_block(s, x, y);
            -- (*interactionLevel);

            return 2;
        }
        return 0;

    // Key hole
    case 10:

        if ((*keyCount) > 0 &&
            actionType == STATE_PRESSED) {

            open_lock(s, x, y);
            -- (*keyCount);

            return 2;
        }
        return 0;

    default:
        break;
    }

    return (u8)!is_solid(id);
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

        tmap_clone_area(s->baseMap, 
            s->roomTilesStatic, 0, 
            s->camPos.x, s->camPos.y, 
            s->roomWidth, s->roomHeight);
        memcpy(s->roomTilesDynamic, s->roomTilesStatic, 
            s->roomWidth * s->roomHeight);

        gen_wall_tile_map(s);

        memset(s->renderBuffer, 1, s->roomWidth * s->roomHeight);

        return true;
    }
    return false;
}


u8 stage_check_overlay(Stage* s, i16 x, i16 y) {

    u8 v = get_tile(s, s->roomTilesStatic, x, y, 0);

    if (is_item(v)) {

        if (v == 17)
            set_tile_permanent(s, x, y, 0);
        else
            set_tile_both(s, x, y, 0);

        return v - 16;
    }

    return 0;
}
