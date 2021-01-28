#include "game.h"

#include "err.h"
#include "graph.h"
#include "keyb.h"
#include "sprite.h"
#include "tilemap.h"
#include "stage.h"
#include "util.h"

#include <stdlib.h>


//
// In general one should not use global
// variables for anything, but we might win
// something here
//
static bool cleared = false;

static Tilemap* baseMap;
static Bitmap* bmpFont = NULL;
static Bitmap* bmpFigure = NULL;
static Bitmap* bmpTileset = NULL;

static Stage* gameStage;


bool init_game_scene() {

    cleared = false;

    if ((baseMap = load_tilemap("ASSETS/MAP.BIN")) == NULL) {

        return true;
    }

    if ((bmpFont = load_bitmap("ASSETS/FONT.BIN")) == NULL ||
        (bmpFigure = load_bitmap("ASSETS/FIGURE.BIN")) == NULL ||
        (bmpTileset = load_bitmap("ASSETS/TILESET.BIN")) == NULL) {

        return true;
    }

    gameStage = new_stage(baseMap, 12, 10, 0, 0);
    if (gameStage == NULL) {

        return true;
    }

    return false;
}



bool game_refresh(i16 step) {

    if (keyb_get_normal_key(KEY_Q) == STATE_PRESSED &&
        (keyb_get_normal_key(KEY_LCTRL) & STATE_DOWN_OR_PRESSED)) {

        return true;
    }

    return false;
}


void game_redraw() {

    if (!cleared) {

        clear_screen(1);
        cleared = true;
    }

    stage_draw(gameStage, bmpTileset);

    draw_text(bmpFont, "Hello CGA!", 1, 4, false);
}


void dispose_game_scene() {

    dispose_bitmap(bmpFont);
    dispose_bitmap(bmpFigure);
    dispose_bitmap(bmpTileset);

    dispose_tilemap(baseMap);
    
    dispose_stage(gameStage);
}
