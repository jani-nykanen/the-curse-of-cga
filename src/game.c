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


//
// In general one should not use global
// variables for anything, but we might win
// something here
//
static bool bgDrawn = false;

static Tilemap* baseMap;
static Bitmap* bmpFont = NULL;
static Bitmap* bmpFigure = NULL;
static Bitmap* bmpTileset = NULL;
static Bitmap* bmpHUD = NULL;
static Bitmap* bmpObjects = NULL;
static Bitmap* bmpLogo = NULL;

static Stage* gameStage;
static Player* player;


static void draw_frame(Bitmap* bmp, i16 x, i16 y, i16 w, i16 h, i16 xshift, bool drawShadow) {

    i16 i;
    i16 end;

    // Horizontal
    end = w / 2;
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


bool init_game_scene() {

    bgDrawn = false;

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

    return false;
}



bool game_refresh(i16 step) {

    pl_update(player, gameStage, step);
    stage_update(gameStage, step);

    if (keyb_get_normal_key(KEY_Q) == STATE_PRESSED &&
        (keyb_get_normal_key(KEY_LCTRL) & STATE_DOWN_OR_PRESSED)) {

        return true;
    }

    return false;
}


static void draw_map(i16 x, i16 y) {

    const i16 WIDTH = 16;
    const i16 HEIGHT = 56;

    draw_frame(bmpHUD, x, y, WIDTH, HEIGHT, 6, false);
    fill_rect(x, y, WIDTH, HEIGHT, 0);
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


static void draw_hud() {

    clear_screen(1);
        draw_frame(bmpHUD, gameStage->xoff, gameStage->yoff,
            gameStage->roomWidth*4,
            gameStage->roomHeight*16, 
            0, true);

    // Logo
    draw_bitmap_fast(bmpLogo, 80 - 21, 8);

    // Map
    draw_map(80-19, 84);
    draw_text(bmpFont, "MAP:", 80-10, 71, true);

    // Treasure & items
    draw_text(bmpFont, "TREASURE:", 80-11, 148, true);
    draw_treasure(80- 21, 158);
}


void game_redraw() {

    if (!bgDrawn) {

        draw_hud();
        bgDrawn = true;
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
}
