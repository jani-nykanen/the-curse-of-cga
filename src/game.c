#include "game.h"

#include "err.h"
#include "graph.h"
#include "keyb.h"

#include <stdlib.h>


// TODO: This might lead to stack overflow.
// If that happens, make every variable global, even
// though that is waste of memory
typedef struct {

    bool cleared; 

} _GameState;


static bool init_game_state(_GameState* state) {

    state->cleared = false;

    return false;
}


bool game_refresh(void* pself, i16 step) {

    _GameState* self = (_GameState*)pself;

    if (keyb_get_normal_key(KEY_Q) == STATE_PRESSED &&
        (keyb_get_normal_key(KEY_LCTRL) & STATE_DOWN_OR_PRESSED)) {

        return true;
    }

    return false;
}


void game_redraw(void* pself) {

    _GameState* self = (_GameState*)pself;

    if (!self->cleared) {

        clear_screen(2);
        self->cleared = true;
    }
}


GameState new_game_state() {

    _GameState* state = (_GameState*) malloc(sizeof(_GameState));
    if (state == NULL) {

        ERR_MALLOC();
        return NULL;
    }

    // Initialize
    if (init_game_state(state)) {

        free(state);
        return NULL;
    }

    return (GameState)state;
}


void dispose_game_state(GameState _state) {

    _GameState* state = (_GameState*)_state;

    // Heh, free state
    free(state);
}
