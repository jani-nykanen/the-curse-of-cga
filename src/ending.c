#include "ending.h"

#include "graph.h"
#include "util.h"
#include "keyb.h"
#include "title.h"
#include "core.h"

#include <stdlib.h>


static const i16 CLEAR_TIME = 60;


static Bitmap* bmpEnding = NULL;

static i16 clearTimer;
static i16 charTimer;
static u8 charCount;
static u8 lastChar;
static i16 oldRow;


bool init_ending_scene() {

    clearTimer = CLEAR_TIME;
    charTimer = 0;
    charCount = 0;
    lastChar = 255;
    oldRow = -1;

    if ((bmpEnding = load_bitmap("ASSETS/ENDING.BIN")) == NULL) {

        return true;
    }

    return false;
}


bool ending_refresh(i16 step) {

    const u8 LAST_CHAR = 7;
    const i16 CHAR_TIME = 10;

    if (clearTimer >= 0) {

        clearTimer -= step;
        return false;
    }

    lastChar = charCount;
    if (charCount < LAST_CHAR) {

        if ((charTimer += step) >= CHAR_TIME) {

            ++ charCount;
            charTimer -= CHAR_TIME;
        }
    }
    else if (keyb_any_pressed()) {

        if (init_title_screen_scene()) {

            return true;
        }

        core_register_callbacks(
            title_screen_refresh, 
            title_screen_redraw);
    }

    return false;
}


static void clear_to_blue() {

    i16 i;
    i16 h = ((CLEAR_TIME - clearTimer) << 4) / CLEAR_TIME * 100;
    h >>= 4;

    h = fixed_round(h, 10);

    if (h != oldRow && h >= 0) {

        for (i = 0; i < 20; ++ i) {

            draw_bitmap_region(bmpEnding, 24, 0, 4, 16, i*4, h*10);
            draw_bitmap_region(bmpEnding, 24, 0, 4, 16, i*4, 192 - h*10);
        }
    }

    oldRow = h;
}


void ending_redraw() {

    i16 dx;
    i16 sx;

    if (clearTimer >= 0) {

        clear_to_blue();
        return;
    }  

    if (charCount != lastChar && charCount != 3) {

        sx = charCount < 3 ? (charCount * 4) : (charCount-1)*4;
        dx = 40 - 16 + charCount * 4;

        draw_bitmap_region(bmpEnding, sx, 0, 4, 16, dx, 100-8);
    }
}


void dispose_ending_scene() {

    dispose_bitmap(bmpEnding);
}
