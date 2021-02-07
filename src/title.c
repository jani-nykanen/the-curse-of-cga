#include "title.h"

#include "graph.h"
#include "err.h"
#include "menu.h"
#include "game.h"
#include "core.h"
#include "keyb.h"

#include <stdio.h>
#include <stdlib.h>


static Bitmap* bmpLogo = NULL;
static Bitmap* bmpFont = NULL;

static Menu* menu;

static bool bgDrawn;
static i16 buttonPressed;
static i8 titlePhase = -1;
static i16 flickerTimer;


static void menu_callback(i16 button) {

    buttonPressed = button + 1;
}


bool init_title_screen_scene() {

    const str TITLE_BUTTON_NAMES[] = {
        "START GAME",
        "QUIT GAME"
    };

    bgDrawn = false;
    buttonPressed = 0;
    titlePhase = titlePhase < 0 ? 0 : 1;
    flickerTimer = 29;

    if ((bmpFont = load_bitmap("ASSETS/FONT.BIN")) == NULL ||
        (bmpLogo = load_bitmap("ASSETS/TITLE.BIN")) == NULL) {

        return true;
    }

    menu = new_menu(TITLE_BUTTON_NAMES, menu_callback, 2);
    if (menu == NULL) {

        return true;
    }
    menu_activate(menu, 0);

    return false;
}


bool title_screen_refresh(i16 step) {

    if (titlePhase == 0) {

        flickerTimer = (flickerTimer + step) % 60;
        if (keyb_get_normal_key(KEY_RETURN) == STATE_PRESSED) {

            titlePhase = 1;
        }
        return false;
    }

    if (buttonPressed != 0) {

        if (buttonPressed == 2) {

            return true;
        }
        else {

            if (init_game_scene()) {

                return true;
            }
            core_register_callbacks(game_refresh, game_redraw);
        }
    }

    menu_update(menu, step);

    return false;
}


void title_screen_redraw() {

    if (!bgDrawn) {

        clear_screen(2);

        draw_bitmap_fast(bmpLogo, 40 - bmpLogo->width/8, 16);
        draw_text(bmpFont, "A \"FINISHED PROTOTYPE\"",
            40, 104, -1, true);

        draw_text(bmpFont, "@2021 Jani Nyk~nen",
            40, 190, -1, true);

        bgDrawn = true;
    }

    if (titlePhase == 0) {

        if (flickerTimer < 30)
            fill_rect(28, 144, 24, 8, 2);
        else
            draw_text(bmpFont, "PRESS ENTER", 40, 144, -1, true);
    }
    else {

        menu_draw(menu, bmpFont, 40, 144);
    }
}


void dispose_title_screen_scene() {

    dispose_bitmap(bmpLogo);
    dispose_bitmap(bmpFont);

    dispose_menu(menu);
}
