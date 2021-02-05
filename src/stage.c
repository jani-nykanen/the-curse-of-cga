#include "stage.h"

#include "graph.h"
#include "util.h"
#include "err.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const u8 ITEM_START_INDEX = 33;

static const i16 DISAPPEAR_TIME = 12;

static const i16 ARROW_DIRX[] = {0, 0, -1, 1};
static const i16 ARROW_DIRY[] = {-1, 1, 0, 0};


static bool is_solid(u8 v) {

    static const bool TABLE[] = {
        0, 1, 1, 1, 
        0, 1, 1, 1, 
        1, 1, 1, 0, 
        0, 0, 0, 1,
        1, 1, 1, 1,

        // "Overflow" values, remove in the
        // release version
        0, 0, 0, 0, 
        0, 0, 0, 0,
        0, 0, 0, 0,
    };

    return v < ITEM_START_INDEX && TABLE[(i16)v];
}


static bool is_solid_ignore_water(u8 v) {

    if (v == 3)
        return false;

    return is_solid(v);
}


static bool is_item(u8 v) {

    return v >= ITEM_START_INDEX;
}


static u8 get_tile(Stage* s, u8* arr, i16 x, i16 y, u8 def) {

    if (x < 0 || y < 0 || x >= s->roomWidth || y >= s->roomHeight)
        return def;

    return arr[y * s->roomWidth + x];
}

/*
static u8 get_tile_either(Stage* s, i16 x, i16 y, u8 def) {

    u8 id = get_tile(s, s->roomTilesStatic, x, y, def);
    if (id == 0)
        return get_tile(s, s->roomTilesDynamic, x, y, def);

    return id;
}
*/


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
    tmap_set_tile(s->baseMap, 0, 
        s->camPos.x + x, s->camPos.y + y, v);
}


static void start_disappear_animation(Stage* s, u8 tile, i16 x, i16 y) {

    s->disappearTimer = DISAPPEAR_TIME;
    s->disappearTile = tile;
    s->disappearPos = vec2(x, y);
}


static i16 move_boulder(Stage* s, i16 x, i16 y, i16 dx, i16 dy, i16 objectMoveTime) {

    i16 tid = get_tile(s, s->roomTilesDynamic, x+dx, y+dy, 1);
    if (tid != 255 &&
        !is_solid_ignore_water(tid)) {

        set_tile(s, s->roomTilesDynamic, x, y, 0);
        set_tile(s, s->roomTilesDynamic, x+dx, y+dy, 2);

        s->rockAnim->pos = vec2(x, y);
        s->rockAnim->target = vec2(x+dx, y+dy);
        s->rockAnim->startTime = objectMoveTime;
        s->rockAnim->timer = objectMoveTime;

        return 1;
    }
    return 0;
}


static bool check_confict(Stage* s, i16 x, i16 y) {

    u8 tid;

    // Rock and something
    if (get_tile(s, s->roomTilesDynamic, x, y, 0) == 2) {

        tid = get_tile(s, s->roomTilesStatic, x, y, 0);

        // Rock & water
        if (tid == 3) {

            set_tile_both(s, x, y, 0);
            start_disappear_animation(s, 22, x, y);

            return true;
        }
        // Rock & automatic arrows
        else if (tid >= 11 && tid <= 14) {

            return move_boulder(s, x, y, 
                ARROW_DIRX[tid-11], ARROW_DIRY[tid-11], 
                s->rockAnim->startTime) == 1;
        }
    }
    return false;
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
    i16 roomWidth, i16 roomHeight, Vector2* startPos) {

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

    *startPos = stage_find_player(s);

    // Static tiles for objects that cannot be moved,
    // and that can be overlaid
    s->roomTilesStatic = (u8*)malloc(roomWidth * roomHeight);
    if (s->roomTilesStatic == NULL) {

        dispose_stage(s);
        return NULL;
    }
    tmap_clone_area(baseMap, s->roomTilesStatic,
        0, s->camPos.x, s->camPos.y, roomWidth, roomHeight);

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

    s->rockAnim = (MovingRock*)malloc(sizeof(MovingRock));
    if (s->rockAnim == NULL) {

        ERR_MALLOC();
        dispose_stage(s);
        return NULL;
    }

    s->rockAnim->timer = 0;

    s->disappearPos = vec2(0, 0);
    s->disappearTimer = 0;
    s->disappearTile = 0;

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
    if (s->rockAnim != NULL)
        free(s->rockAnim);

    free(s);
}


bool stage_update(Stage* s, i16 step) {

    i16 moveStep;
    i16 delta;

    // Animate rock
    if (s->rockAnim->timer > 0) {

        stage_mark_tile_for_redraw(s, 
                s->rockAnim->pos.x, 
                s->rockAnim->pos.y);
        stage_mark_tile_for_redraw(s, 
                s->rockAnim->target.x, 
                s->rockAnim->target.y);

        s->rockAnim->timer -= step;
        if (s->rockAnim->timer <= 0) {

            return check_confict(s, s->rockAnim->target.x, s->rockAnim->target.y);
        }

        moveStep = s->rockAnim->startTime / 4;
        delta = 4 - fixed_round(s->rockAnim->timer, moveStep);

        s->rockAnim->rpos.x = s->rockAnim->pos.x * 4 + 
            (s->rockAnim->target.x - s->rockAnim->pos.x) * delta;
        s->rockAnim->rpos.y = s->rockAnim->pos.y * 16 + 
            (s->rockAnim->target.y - s->rockAnim->pos.y) * delta * 4;
    }

    // Animate disappearing tile
    if (s->disappearTimer > 0) {

        -- s->disappearTimer;
        stage_mark_tile_for_redraw(s, s->disappearPos.x, s->disappearPos.y);
    }

    return false;
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

    const i16 MAX_ID = 19;
    static const i16 SRCX[] = {
        16, 0, 16, 16, 
        0, 4, 8, 12, 
        20, 20, 24, 28, 
        32, 28, 32, 36, 
        36, 36, 40, 40
    };

    static const i16 SRCY[] = {
        0, 0, 0, 16,
        16, 16, 16, 16,
        0, 16, 0, 0, 
        0, 16, 16, 0,
        16, 32, 0, 16
    };

    i16 x, y;
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

            if (tid <= MAX_ID) {

                if (tid == 1) {

                    stage_draw_wall(s, bmpTileset, x, y);
                    continue;
                }
                else {

                    // Type-casting to get rid of warnings
                    sx = SRCX[(u16)tid];
                    sy = SRCY[(u16)tid];
                }
            }
            // Items
            else if (tid >= ITEM_START_INDEX) {

                sx = 4 * (tid - ITEM_START_INDEX);
                sy = 32;
            }
            else {

                sx = SRCX[0];
                sy = SRCY[0];
            }

            draw_bitmap_region_fast(bmpTileset,
                sx, sy, 4, 16, 
                s->xoff + x*4, 
                s->yoff + y*16);
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

                if (s->rockAnim->timer <= 0 ||
                    s->rockAnim->target.x != x ||
                    s->rockAnim->target.y != y) {

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

    if (s->rockAnim->timer > 0) {

        draw_bitmap_region(bmpObjects,
            0, 0, 4, 16, 
            s->xoff + s->rockAnim->rpos.x,
            s->yoff + s->rockAnim->rpos.y);
    }
}


void stage_draw_effects(Stage* s, Bitmap* bmpTileset) {

    i16 sx, sy;
    i16 skip;
    const i16 m = DISAPPEAR_TIME / 2;

    if (s->disappearTimer <= 0) return;

    sx = (i16)(s->disappearTile % 16);
    sx *= 4;
    sy = (i16)(s->disappearTile / 16);
    sy *= 16;

    if (s->disappearTimer >= m) {

        skip = 1 + (s->disappearTimer - m);
    }
    else {

        skip = -1 -(m - s->disappearTimer);
    }

    draw_bitmap_region_fast_skip_lines(bmpTileset, sx, sy, 4, 16,
        s->xoff + s->disappearPos.x * 4,
        s->yoff + s->disappearPos.y * 16, 
        skip);
}


void stage_mark_tile_for_redraw(Stage* s, i16 x, i16 y) {

    if (x < 0 || y < 0 || x >= s->roomWidth || y >= s->roomHeight)
        return;

    s->renderBuffer[y * s->roomWidth + x] = true;
}


void stage_redraw_all(Stage* s) {

    memset(s->renderBuffer, 1, s->roomWidth * s->roomHeight);
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

    start_disappear_animation(s, 5, x, y);
}


static void break_ice_block(Stage* s, i16 x, i16 y) {

    set_tile(s, s->roomTilesStatic, x, y, 0);
    set_tile(s, s->roomTilesDynamic, x, y, 2);

    start_disappear_animation(s, 21, x, y);
}


static void open_lock(Stage* s, i16 x, i16 y) {

    set_tile_permanent(s, x, y, 0);
    start_disappear_animation(s, 6, x, y);
}


static void remove_rubble(Stage* s, i16 x, i16 y) {

    set_tile(s, s->roomTilesStatic, x, y, 3);
    set_tile(s, s->roomTilesDynamic, x, y, 3);

    start_disappear_animation(s, 48, x, y);
}


static void set_gem(Stage* s, i16 x, i16 y) {

    set_tile_permanent(s, x, y, 19);
}



static void swap_automatic_arrows(Stage* s, i16 dx, i16 dy, u8 v) {

    static const u8 NEW_VALUE[] = {12, 11, 14, 13};

    i16 x, y;
    u8 tid;

    // Swap walls
    for (y = 0; y < s->roomHeight; ++ y) {

        for (x = 0; x < s->roomWidth; ++ x) {

            tid = get_tile(s, s->roomTilesStatic, x, y, 0);
            if (tid >= 11 && tid <= 14) {

                set_tile_both(s, x, y, NEW_VALUE[tid - 11]);
            }
        }
    }
    set_tile_both(s, dx, dy, v == 15 ? 16 : 15);
}


u8 stage_movement_collision(Stage* s, 
    State actionType, i16 x, i16 y, 
    i16 dx, i16 dy, i16 objectMoveTime, u8* access,
    u8* interactionLevel, u8* keyCount, u8* gemCount) {

    u8 id = get_tile(s, s->roomTilesDynamic, x, y, 0);

    switch (id) {

    // Rock
    case 2:

        if (access == NULL || access[0])
            return move_boulder(s, x, y, dx, dy, objectMoveTime);
        return 0;

    // Bolt
    case 6:

        if (access != NULL &&
            access[1] && 
            actionType == STATE_PRESSED) {

            swap_walls(s, x, y);
            return 2;
        }
        return 0;

    // Flower
    case 8:

        if (access != NULL && 
            access[3] && 
            (*interactionLevel) > 0 && 
            actionType == STATE_PRESSED) {

            cut_flower(s, x, y);
            -- (*interactionLevel);

            return 2;
        }
        return 0;

    // Ice block
    case 9:

        if (access != NULL && 
            access[4] &&
            (*interactionLevel) > 0 &&
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

    // That spinning things
    case 15:
    case 16:

        if (access != NULL && 
            access[2] &&
            actionType == STATE_PRESSED) {

            swap_automatic_arrows(s, x, y, id);
            return 2;
        }
        return 0;

    // Rubble
    case 17:

        if (access != NULL && 
            access[5] &&
            (*interactionLevel) > 0 &&
            actionType == STATE_PRESSED) {

            remove_rubble(s, x, y);
            -- (*interactionLevel);

            return 2;
        }
        return 0;

    // Gem holder
    case 18:

        if ((*gemCount) > 0 &&
            actionType == STATE_PRESSED) {

            set_gem(s, x, y);
            -- (*gemCount);

            return 2;
        }
        return 0;

    default:
        break;
    }

    return (u8)!is_solid(id);
}



u8 stage_check_automatic_movement(Stage* s, i16 x, i16 y, Vector2* target) {

    i16 tid = (i16)get_tile(s, s->roomTilesStatic, x, y, 0);
    if (tid >= 11 && tid <= 14) {

        tid -= 11;

        if (stage_movement_collision(s, 
            STATE_DOWN, x, y, 
            ARROW_DIRX[tid], 
            ARROW_DIRY[tid], 
            0, NULL, NULL, 
            NULL, NULL) == 1) {

            target->x = x + ARROW_DIRX[tid];
            target->y = y + ARROW_DIRY[tid];

            return 1;
        }
    }
    return 0;
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

        s->disappearTimer = 0;

        return true;
    }
    return false;
}


u8 stage_check_overlay(Stage* s, i16 x, i16 y) {

    u8 v = get_tile(s, s->roomTilesStatic, x, y, 0);

    if (is_item(v)) {

        if (v == ITEM_START_INDEX+1)
            set_tile_both(s, x, y, 0);
        else
            set_tile_permanent(s, x, y, 0);

        return v - (ITEM_START_INDEX-1);
    }

    return 0;
}


bool stage_check_conflict(Stage* s, i16 x, i16 y) {

    return is_solid(get_tile(s, s->roomTilesDynamic, x, y, 0));
}


void stage_mark_tile_solid(Stage* s, i16 x, i16 y, bool state) {

    set_tile(s, s->roomTilesDynamic, x, y, 255 * (u8)state);
}


void stage_reset_room(Stage* s) {

    tmap_clone_area(s->baseMap, 
            s->roomTilesStatic, 0, 
            s->camPos.x, s->camPos.y, 
            s->roomWidth, s->roomHeight);
    memcpy(s->roomTilesDynamic, s->roomTilesStatic, 
        s->roomWidth * s->roomHeight);
    memset(s->renderBuffer, 1, s->roomWidth * s->roomHeight);

    s->rockAnim->timer = 0;
    s->disappearTimer = 0;
}


void stage_flush_redraw_buffer(Stage* s) {

    memset(s->renderBuffer, 0, s->roomWidth * s->roomHeight);
}


void stage_partial_redraw(Stage* s, i16 row) {

    if (row < 0 || row >= s->roomHeight) return;

    memset(s->renderBuffer + (u32)(row * s->roomWidth), 1, s->roomWidth);
}


Vector2 stage_find_player(Stage* s) {

    i16 x, y;

    for (y = 0; y < s->baseMap->height; ++ y) {

        for (x = 0; x < s->baseMap->width; ++ x) {

            if (tmap_get_tile(s->baseMap, 0, x, y, 0) == 32) {

                s->camPos = vec2(x / s->roomWidth, y / s->roomHeight);
                s->camPos.x *= s->roomWidth;
                s->camPos.y *= s->roomHeight;

                return vec2(x % s->roomWidth, y % s->roomHeight);
            }
        }
    }

    return vec2(0, 0);
}
