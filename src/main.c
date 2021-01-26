#include "core.h"
#include "graph.h"
#include "err.h"

#include "game.h"

#include <stdio.h>


static void print_errors() {

    char buffer [ERROR_MAX_LENGTH];
    
    core_dispose();
    while (err_pop(buffer)) {

        printf("ERROR: %s\n", buffer);
    }
}


void main() {

    GameState* state;

    init_core();

    state = new_game_state();
    if (state == NULL) {

        print_errors();
        return;
    }

    core_register_callbacks(
        (void*)state,
        game_refresh,
        game_redraw);

    core_run(1);
}
