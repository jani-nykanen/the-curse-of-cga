#include "core.h"
#include "graph.h"
#include "keyb.h"

#include <stdlib.h>


static u16 frameCounter = 0;
static CoreCallbackRefresh cbRefresh = NULL;
static CoreCallbackRedraw cbRedraw = NULL;


static void destroy_core() {

    reset_graphics();
    reset_keyboard_listener();
}


static bool main_loop(i16 frameSkip) {

    if ( (frameCounter ++) == frameSkip) {

        frameCounter = 0;
    
        if (cbRefresh != NULL) {

            if (cbRefresh(frameSkip+1))
                return true;
        }

        vblank();

        if (cbRedraw != NULL) {

            cbRedraw();
        }
    }
    else {

        vblank();
    }

    return false;
}


void init_core() {

    init_graphics();
    init_keyboard_listener();
}


void core_register_callbacks(
    CoreCallbackRefresh refresh,
    CoreCallbackRedraw redraw) {

    cbRefresh = refresh;
    cbRedraw = redraw;
}


void core_run(i16 frameSkip) {

    frameCounter = 0;
    while (!main_loop(frameSkip));

    destroy_core();
}


void core_dispose() {

    destroy_core();
}
