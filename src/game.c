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

static Stage* gameStage;
static Player* player;


static void draw_frame(Bitmap* bmp, i16 x, i16 y, i16 w, i16 h) {

    i16 i;
    i16 end;

    // Horizontal
    end = w / 2;
    for (i = 0; i < end + 2; ++ i) {

        if (i < end) {

            draw_bitmap_region_fast(bmp, 2, 0, 2, 8, x + i*2, y - 8);
            draw_bitmap_region_fast(bmp, 2, 16, 2, 8, x + i*2, y + h);
        }
        draw_bitmap_region_fast(bmp, 2, 8, 2, 8, x + i*2, y + h + 8);
    }

    // Vertical
    end = h / 8;
    for (i = 0; i < end + 1; ++ i) {

        if (i < end) {
            
            draw_bitmap_region_fast(bmp, 0, 8, 2, 8, x - 2, y + i*8);
            draw_bitmap_region_fast(bmp, 4, 8, 2, 8, x + w, y + i*8);
        }
        
        draw_bitmap_region_fast(bmp, 2, 8, 2, 8, x + w + 2, y + i*8);
    }

    // Corners
    draw_bitmap_region_fast(bmp, 0, 0, 2, 8, x - 2, y - 8);   
    draw_bitmap_region_fast(bmp, 4, 0, 2, 8, x + w, y - 8);   
    draw_bitmap_region_fast(bmp, 0, 16, 2, 8, x - 2, y + h);   
    draw_bitmap_region_fast(bmp, 4, 16, 2, 8, x + w, y + h);  
}


bool init_game_scene() {

    bgDrawn = false;

    if ((baseMap = load_tilemap("ASSETS/MAP.BIN")) == NULL) {

        return true;
    }

    if ((bmpFont = load_bitmap("ASSETS/FONT.BIN")) == NULL ||
        (bmpFigure = load_bitmap("ASSETS/FIGURE.BIN")) == NULL ||
        (bmpTileset = load_bitmap("ASSETS/TILESET.BIN")) == NULL ||
        (bmpHUD = load_bitmap("ASSETS/HUD.BIN")) == NULL) {

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

    if (keyb_get_normal_key(KEY_Q) == STATE_PRESSED &&
        (keyb_get_normal_key(KEY_LCTRL) & STATE_DOWN_OR_PRESSED)) {

        return true;
    }

    return false;
}


void game_redraw() {

    if (!bgDrawn) {

        clear_screen(1);
        draw_frame(bmpHUD, gameStage->xoff, gameStage->yoff,
            gameStage->roomWidth*4,
            gameStage->roomHeight*16);

        bgDrawn = true;
    }

    pl_update_stage_tile_buffer(player, gameStage);
    stage_draw(gameStage, bmpTileset);

    pl_draw(player, bmpFigure);

    //draw_text(bmpFont, "Hello CGA!", 1, 4, false);
}


void dispose_game_scene() {

    dispose_bitmap(bmpFont);
    dispose_bitmap(bmpFigure);
    dispose_bitmap(bmpTileset);
    dispose_bitmap(bmpHUD);

    dispose_tilemap(baseMap);
    
    dispose_stage(gameStage);

    if (player != NULL)
        free(player);
}
