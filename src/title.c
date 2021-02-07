#include "title.h"

#include "graph.h"
#include "err.h"

#include <stdio.h>
#include <stdlib.h>


static Bitmap* bmpLogo = NULL;
static Bitmap* bmpFont = NULL;

static bool bgDrawn;


bool init_title_screen_scene() {

    bgDrawn = false;

    if ((bmpFont = load_bitmap("ASSETS/FONT.BIN")) == NULL ||
        (bmpLogo = load_bitmap("ASSETS/TITLE.BIN")) == NULL) {

        return true;
    }

    return false;
}


bool title_screen_refresh(i16 step) {

    return false;
}


void title_screen_redraw() {

    if (!bgDrawn) {

        clear_screen(2);
        draw_bitmap_fast(bmpLogo, 40 - bmpLogo->width/8, 16);
        draw_text(bmpFont, "A \"FINISHED PROTOTYPE\"",
            40, 104, -1, true);
    }
}


void dispose_title_screen_scene() {

    dispose_bitmap(bmpLogo);
    dispose_bitmap(bmpFont);
}
