#include "game.h"

#include "err.h"
#include "graph.h"
#include "keyb.h"
#include "sprite.h"
#include "tilemap.h"
#include "stage.h"
#include "util.h"
#include "player.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


static const i16 MAP_X = 80-19;
static const i16 MAP_Y = 77;
// TODO: In future, fetch the following info
// from the base map itself
static const i16 ROOM_COUNT_X = 5;
static const i16 ROOM_COUNT_Y = 5;
static const i16 MAP_WIDTH = 15;
static const i16 MAP_HEIGHT = 48;



//
// In general one should not use global
// variables for anything, but we might win
// something here
//
static bool bgDrawn = false;
static bool mapDrawn = false;

static Tilemap* baseMap;
static Bitmap* bmpFont = NULL;
static Bitmap* bmpFigure = NULL;
static Bitmap* bmpTileset = NULL;
static Bitmap* bmpHUD = NULL;
static Bitmap* bmpObjects = NULL;
static Bitmap* bmpLogo = NULL;

static Stage* gameStage;
static Player* player;

static u8* visitedRooms;


static void draw_frame(Bitmap* bmp, i16 x, i16 y, i16 w, i16 h, 
    i16 xshift, i16 wextra, bool drawShadow) {

    i16 i;
    i16 end;

    // Horizontal
    end = w / 2 + wextra;
    for (i = 0; i < end + ((i16)drawShadow) * 2; ++ i) {

        if (i < end) {

            draw_bitmap_region_fast(bmp, xshift + 2, 0, 2, 8, x + i*2, y - 8);
            draw_bitmap_region_fast(bmp, xshift + 2, 16, 2, 8, x + i*2, y + h);
        }

        if (drawShadow)
            draw_bitmap_region_fast(bmp, xshift + 2, 8, 2, 8, x + i*2, y + h + 8);
    }

    // Vertical
    end = h / 8;
    for (i = 0; i < end + ((i16)drawShadow); ++ i) {

        if (i < end) {
            
            draw_bitmap_region_fast(bmp, xshift, 8, 2, 8, x - 2, y + i*8);
            draw_bitmap_region_fast(bmp, xshift + 4, 8, 2, 8, x + w, y + i*8);
        }

        if (drawShadow)
            draw_bitmap_region_fast(bmp, xshift + 2, 8, 2, 8, x + w + 2, y + i*8);
    }

    // Corners
    draw_bitmap_region_fast(bmp, xshift + 0, 0, 2, 8, x - 2, y - 8);   
    draw_bitmap_region_fast(bmp, xshift + 4, 0, 2, 8, x + w, y - 8);   
    draw_bitmap_region_fast(bmp, xshift + 0, 16, 2, 8, x - 2, y + h);   
    draw_bitmap_region_fast(bmp, xshift + 4, 16, 2, 8, x + w, y + h);  
}


static void mark_room_visited() {

    i16 cx = gameStage->camPos.x / gameStage->roomWidth;
    i16 cy = gameStage->camPos.y / gameStage->roomHeight;

    visitedRooms[cy * ROOM_COUNT_X + cx] = true;
}


bool init_game_scene() {

    bgDrawn = false;
    mapDrawn = false;

    if ((baseMap = load_tilemap("ASSETS/MAP.BIN")) == NULL) {

        return true;
    }

    if ((bmpFont = load_bitmap("ASSETS/FONT.BIN")) == NULL ||
        (bmpFigure = load_bitmap("ASSETS/FIGURE.BIN")) == NULL ||
        (bmpTileset = load_bitmap("ASSETS/TILESET.BIN")) == NULL ||
        (bmpHUD = load_bitmap("ASSETS/HUD.BIN")) == NULL ||
        (bmpObjects = load_bitmap("ASSETS/OBJECTS.BIN")) == NULL ||
        (bmpLogo = load_bitmap("ASSETS/LOGO.BIN")) == NULL) {

        return true;
    }

    gameStage = new_stage(baseMap, 12, 10, 0, 0);
    if (gameStage == NULL) {

        return true;
    }

    alloc_object(player, Player, true);
    *player = create_player(1, 1, gameStage);

    visitedRooms = (u8*)malloc(ROOM_COUNT_X * ROOM_COUNT_Y);
    if (visitedRooms == NULL) {

        return true;
    }
    memset(visitedRooms, 0, ROOM_COUNT_X * ROOM_COUNT_Y);
    mark_room_visited();

    return false;
}


bool game_refresh(i16 step) {

    if (pl_update(player, gameStage, step)) {

        mark_room_visited();
        mapDrawn = false;
    }
    stage_update(gameStage, step);

    if (keyb_get_normal_key(KEY_Q) == STATE_PRESSED &&
        (keyb_get_normal_key(KEY_LCTRL) & STATE_DOWN_OR_PRESSED)) {

        return true;
    }

    return false;
}


static void draw_map_frame(i16 x, i16 y) {


    draw_frame(bmpHUD, x, y, MAP_WIDTH, MAP_HEIGHT, 6, 1, false);
    fill_rect(x, y, MAP_WIDTH, MAP_HEIGHT, 0);

    draw_text(bmpFont, "MAP:", 80-11, y-15, true);
}


static void draw_map(i16 x, i16 y) {

    const i16 ROOM_WIDTH = 3;
    const i16 ROOM_HEIGHT = 10;

    i16 dx, dy;
    i16 sy;

    i16 cx = gameStage->camPos.x / gameStage->roomWidth;
    i16 cy = gameStage->camPos.y / gameStage->roomHeight;

    for (dy = 0; dy < ROOM_COUNT_Y; ++ dy) {

        for (dx = 0; dx < ROOM_COUNT_X; ++ dx) {

            if (!visitedRooms[dy * ROOM_COUNT_X + dx])
                continue;

            if (dx == cx && dy == cy) {

                sy = 0;
            }
            else {

                sy = ROOM_HEIGHT;
            }

            draw_bitmap_region_fast(bmpHUD,
                12, sy, ROOM_WIDTH, ROOM_HEIGHT,
                x + dx * ROOM_WIDTH, 
                y + dy * ROOM_HEIGHT);
        }
    }
}


static void draw_treasure(i16 x, i16 y) {

    char buffer[8];

    // Keys
    snprintf(buffer, 8, "\2%d", 0);
    draw_bitmap_region_fast(bmpHUD, 0, 24, 4, 16, x, y);
    draw_text(bmpFont, buffer, x+4, y+4, false);

    // Gems
    snprintf(buffer, 8, "\2%d", 0);
    draw_bitmap_region_fast(bmpHUD, 4, 24, 4, 16, x+10, y);
    draw_text(bmpFont, buffer, x+14, y+4, false);
}


static void draw_battery(i16 x, i16 y) {

    const i16 MAX_BATTERY = 12;
    const i16 TEMP_BATTERY_LEVEL = 3;

    i16 i;
    i16 sx;

    draw_bitmap_region_fast(bmpHUD, 8, 24, 4, 16, x, y);

    for (i = 0; i < MAX_BATTERY; ++ i) {

        sx = (i16)(i < TEMP_BATTERY_LEVEL) * 2;

        draw_bitmap_region_fast(bmpHUD, sx + 12, 24, 2, 16, x + 4+i, y);
    }
}


static void draw_hud() {

    clear_screen(1);
        draw_frame(bmpHUD, gameStage->xoff, gameStage->yoff,
            gameStage->roomWidth*4,
            gameStage->roomHeight*16, 
            0, 0, true);

    // Logo
    draw_bitmap_fast(bmpLogo, 80 - 21, 0);

    // Map
    draw_map_frame(MAP_X, MAP_Y);

    // Treasure & items
    draw_text(bmpFont, "TREASURE:", 80-11, 134, true);
    draw_treasure(80-21, 143);

    // Battery
    draw_text(bmpFont, "BATTERY:", 80-11, 172-8, true);
    draw_battery(80-20, 174);
}


void game_redraw() {

    if (!bgDrawn) {

        draw_hud();
        bgDrawn = true;
    }

    if (!mapDrawn) {

        draw_map(MAP_X, MAP_Y);
        mapDrawn = true;
    }

    pl_update_stage_tile_buffer(player, gameStage);
    stage_draw(gameStage, bmpTileset);
    stage_draw_objects(gameStage, bmpObjects);

    toggle_clipping(true);
    set_clip_rectangle(
        gameStage->xoff, 
        gameStage->yoff,
        gameStage->roomWidth * 4, 
        gameStage->roomHeight*16);

    pl_draw(player, bmpFigure);

    toggle_clipping(false);
}


void dispose_game_scene() {

    dispose_bitmap(bmpFont);
    dispose_bitmap(bmpFigure);
    dispose_bitmap(bmpTileset);
    dispose_bitmap(bmpHUD);
    dispose_bitmap(bmpObjects);
    dispose_bitmap(bmpLogo);

    dispose_tilemap(baseMap);
    
    dispose_stage(gameStage);

    if (player != NULL)
        free(player);

    if (visitedRooms != NULL)
        free(visitedRooms);
}
