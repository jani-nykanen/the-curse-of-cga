#include "game.h"

#include "err.h"
#include "graph.h"
#include "keyb.h"

#include <stdlib.h>


//
// In general one should not use global
// variables for anything, but we might win
// something here
//
static bool cleared = false;
static Vector2 testPos;
static Bitmap* bmpFont = NULL;


bool init_game_scene() {

    cleared = false;
    testPos = vec2(40, 100);

    bmpFont = load_bitmap("ASSETS/FONT.BIN");
    if (bmpFont == NULL) {

        return true;
    }

    return false;
}


bool game_refresh(i16 step) {


    if (keyb_get_ext_key(KEY_UP) & STATE_DOWN_OR_PRESSED) {

        testPos.y -= 4;
    }
    else if (keyb_get_ext_key(KEY_DOWN) & STATE_DOWN_OR_PRESSED) {

        testPos.y += 4;
    }

    if (keyb_get_ext_key(KEY_LEFT) & STATE_DOWN_OR_PRESSED) {

        testPos.x --;
    }
    else if (keyb_get_ext_key(KEY_RIGHT) & STATE_DOWN_OR_PRESSED) {

        testPos.x ++;
    }

    if (keyb_get_normal_key(KEY_Q) == STATE_PRESSED &&
        (keyb_get_normal_key(KEY_LCTRL) & STATE_DOWN_OR_PRESSED)) {

        return true;
    }

    return false;
}


void game_redraw() {

    //if (!cleared) {

        clear_screen(0);
        cleared = true;
    //}

    draw_text_fast(bmpFont, "Hello world!", 1, 4, false);

    fill_rect(
        testPos.x - 4, 
        testPos.y - 13, 
        32 / 4, 26, 2);
}


void dispose_game_scene() {

    dispose_bitmap(bmpFont);
}
