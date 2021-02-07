#include "core.h"
#include "graph.h"
#include "err.h"

#include "title.h"

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

    if (init_title_screen_scene()) {

        print_errors();
        return;
    }

    core_register_callbacks(
        title_screen_refresh,
        title_screen_redraw);

    core_run(1);

    // Assuming that the title screen is the scene
    // where we quit
    dispose_title_screen_scene();
}
