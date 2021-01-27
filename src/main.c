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

    init_core();

    if (init_game_scene()) {

        print_errors();
        return;
    }

    core_register_callbacks(
        game_refresh,
        game_redraw);

    core_run(1);
}
