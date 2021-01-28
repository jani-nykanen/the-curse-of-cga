#include "game.h"

#include "err.h"
#include "graph.h"
#include "keyb.h"
#include "sprite.h"
#include "tilemap.h"

#include <stdlib.h>


//
// In general one should not use global
// variables for anything, but we might win
// something here
//
static bool cleared = false;
static Vector2 testPos;
static Tilemap* baseMap;
static Bitmap* bmpFont = NULL;
static Bitmap* bmpFigure = NULL;
static Bitmap* bmpFace = NULL;
static Sprite testSpr;
static i32 animDir;


bool init_game_scene() {

    cleared = false;
    testPos = vec2(160, 100);
    testSpr = create_sprite(4, 16);
    animDir = -1;

    if ((baseMap = load_tilemap("ASSETS/MAP.BIN")) == NULL) {

        return true;
    }

    if ((bmpFont = load_bitmap("ASSETS/FONT.BIN")) == NULL ||
        (bmpFigure = load_bitmap("ASSETS/FIGURE.BIN")) == NULL ||
        (bmpFace = load_bitmap("ASSETS/FACE.BIN")) == NULL) {

        return true;
    }

    return false;
}


static void test_animation(i16 step) {

    if (animDir == -1) {

        spr_set_frame(&testSpr, 0, testSpr.row);
        return;
    }

    spr_animate(&testSpr, animDir, 0, 3, 8, step);
}


bool game_refresh(i16 step) {

    test_animation(step);

    animDir = -1;
    if (keyb_get_ext_key(KEY_UP) & STATE_DOWN_OR_PRESSED) {

        testPos.y -= 2;
        animDir = 2;
    }
    else if (keyb_get_ext_key(KEY_DOWN) & STATE_DOWN_OR_PRESSED) {

        testPos.y += 2;
        animDir = 0;
    }

    if (keyb_get_ext_key(KEY_LEFT) & STATE_DOWN_OR_PRESSED) {

        testPos.x -= 2;
        animDir = 3;
    }
    else if (keyb_get_ext_key(KEY_RIGHT) & STATE_DOWN_OR_PRESSED) {

        testPos.x += 2;
        animDir = 1;
    }

    if (keyb_get_normal_key(KEY_Q) == STATE_PRESSED &&
        (keyb_get_normal_key(KEY_LCTRL) & STATE_DOWN_OR_PRESSED)) {

        return true;
    }

    return false;
}


void game_redraw() {

    //if (!cleared) {

        clear_screen(2);
        cleared = true;
    //}

    draw_text_fast(bmpFont, "Hello CGA!", 1, 4, false);

    draw_bitmap_fast(bmpFace, 45, 48);
    fill_rect(8, 96, 16, 32, 1);

    draw_sprite(&testSpr, bmpFigure,
        testPos.x/4 - 2, testPos.y-8);
}


void dispose_game_scene() {

    dispose_bitmap(bmpFont);
    dispose_bitmap(bmpFigure);
    dispose_bitmap(bmpFace);

    dispose_tilemap(baseMap);
}
