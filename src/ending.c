#include "ending.h"

#include "graph.h"

#include <stdio.h>


static const i16 CLEAR_TIME = 60;


static i16 clearTimer;


bool init_ending_scene() {

    clearTimer = CLEAR_TIME;

    return false;
}


bool ending_refresh(i16 step) {

    if (clearTimer >= 0) {

        clearTimer -= step;
        return false;
    }

    return false;
}


static void clear_to_blue() {

    i16 h = ((CLEAR_TIME - clearTimer) << 4) / CLEAR_TIME * 100;
    h >>= 4;

    fill_rect(0, 0, 80, h, 1);
    fill_rect(0, 200-1 - h, 80, h, 1);
}


void ending_redraw() {

    if (clearTimer >= 0) {

        clear_to_blue();
        return;
    }
    
}


void dispose_ending_scene() {

}
