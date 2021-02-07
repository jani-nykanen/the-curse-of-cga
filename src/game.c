#include "game.h"

#include "err.h"
#include "graph.h"
#include "keyb.h"
#include "sprite.h"
#include "tilemap.h"
#include "stage.h"
#include "util.h"
#include "player.h"
#include "msgbox.h"
#include "menu.h"
#include "title.h"
#include "core.h"
#include "ending.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>


static const str STORY = 
"Your graphics card is\nbroken and you only\nhave your old CGA\ncard left. You have to\nfind eight magic gems\nin your basement and\n"
"perform a demonic\nritual to get your\ndead card back!";

static const str ENDING = 
"Congratulations!\nWith the power of\ngems, you perform a\ndemonic ritual and\nexchange your soul\nfor a working\ngraphics card.\nHooray!";


static const i16 MAP_X = 80-19;
static const i16 MAP_Y = 77;
// TODO: In future, fetch the following info
// from the base map itself
static const i16 ROOM_COUNT_X = 5;
static const i16 ROOM_COUNT_Y = 5;
static const i16 MAP_WIDTH = 15;
static const i16 MAP_HEIGHT = 48;

static const i16 TRANSITION_TIME = 30;


//
// In general one should not use global
// variables for anything, but we might win
// something here
//
static bool bgDrawn = false;
static bool mapDrawn = false;

static Tilemap* baseMap;
static Bitmap* bmpFont = NULL;
static Bitmap* bmpFigure = NULL;
static Bitmap* bmpTileset = NULL;
static Bitmap* bmpHUD = NULL;
static Bitmap* bmpObjects = NULL;
static Bitmap* bmpLogo = NULL;

static Stage* gameStage;
static Player* player;
static MessageBox* msgBox;
static Menu* pauseMenu;

static u8* visitedRooms;

static u8 oldKeys;
static u8 oldGems;
static u8 oldBatteryLevel;

static i16 transitionTimer;
static u8 transitionMode;
// For faster transition effect rendering
static i16 oldTrHeight;

static bool quit = false;


static void draw_frame(Bitmap* bmp, i16 x, i16 y, i16 w, i16 h, 
    i16 xshift, i16 wextra, bool drawShadow) {

    i16 i;
    i16 end;

    // Horizontal
    end = w / 2 + wextra;
    for (i = 0; i < end + ((i16)drawShadow) * 2; ++ i) {

        if (i < end) {

            draw_bitmap_region_fast(bmp, xshift + 2, 0, 2, 8, x + i*2, y - 8);
            draw_bitmap_region_fast(bmp, xshift + 2, 16, 2, 8, x + i*2, y + h);
        }

        if (drawShadow)
            draw_bitmap_region_fast(bmp, xshift + 2, 8, 2, 8, x + i*2, y + h + 8);
    }

    // Vertical
    end = h / 8;
    for (i = 0; i < end + ((i16)drawShadow); ++ i) {

        if (i < end) {
            
            draw_bitmap_region_fast(bmp, xshift, 8, 2, 8, x - 2, y + i*8);
            draw_bitmap_region_fast(bmp, xshift + 4, 8, 2, 8, x + w, y + i*8);
        }

        if (drawShadow)
            draw_bitmap_region_fast(bmp, xshift + 2, 8, 2, 8, x + w + 2, y + i*8);
    }

    // Corners
    draw_bitmap_region_fast(bmp, xshift + 0, 0, 2, 8, x - 2, y - 8);   
    draw_bitmap_region_fast(bmp, xshift + 4, 0, 2, 8, x + w, y - 8);   
    draw_bitmap_region_fast(bmp, xshift + 0, 16, 2, 8, x - 2, y + h);   
    draw_bitmap_region_fast(bmp, xshift + 4, 16, 2, 8, x + w, y + h);  
}


static void mark_room_visited() {

    i16 cx = gameStage->camPos.x / gameStage->roomWidth;
    i16 cy = gameStage->camPos.y / gameStage->roomHeight;

    visitedRooms[cy * ROOM_COUNT_X + cx] = true;
}


static void pause_menu_callback(i16 cursorPos) {

    switch (cursorPos) {
        
    case 0:
        stage_redraw_all(gameStage);
        break;

    case 1:

        transitionTimer = TRANSITION_TIME;
        transitionMode = 2;
        break;

    case 2:

        transitionTimer = TRANSITION_TIME;
        transitionMode = 3;

        break;

    case 3:

        quit = true;
        break;
    
    default:
        break;
    }

    pauseMenu->active = false;
}


bool init_game_scene() {

    static const str BUTTON_NAMES[] = {
        "RESUME",
        "RESET ROOM",
        "GO TO START",
        "QUIT GAME"
    };

    Vector2 startPos;

    bgDrawn = false;
    mapDrawn = false;

    if ((baseMap = load_tilemap("ASSETS/MAP.BIN")) == NULL) {

        return true;
    }

    if ((bmpFont = load_bitmap("ASSETS/FONT.BIN")) == NULL ||
        (bmpFigure = load_bitmap("ASSETS/FIGURE.BIN")) == NULL ||
        (bmpTileset = load_bitmap("ASSETS/TILESET.BIN")) == NULL ||
        (bmpHUD = load_bitmap("ASSETS/HUD.BIN")) == NULL ||
        (bmpObjects = load_bitmap("ASSETS/OBJECTS.BIN")) == NULL ||
        (bmpLogo = load_bitmap("ASSETS/LOGO.BIN")) == NULL) {

        return true;
    }

    gameStage = new_stage(baseMap, 12, 10, &startPos);
    if (gameStage == NULL) {

        return true;
    }
    stage_flush_redraw_buffer(gameStage);

    pauseMenu = new_menu(BUTTON_NAMES, pause_menu_callback, 4);
    if (pauseMenu == NULL) {

        return true;
    }

    alloc_object(player, Player, true);
    *player = create_player(startPos.x, startPos.y, gameStage);

    alloc_object(msgBox, MessageBox, true);
    *msgBox = create_message_box();

    if (msg_build(msgBox, STORY)) {

        return true;
    }

    visitedRooms = (u8*)malloc(ROOM_COUNT_X * ROOM_COUNT_Y);
    if (visitedRooms == NULL) {

        return true;
    }
    memset(visitedRooms, 0, ROOM_COUNT_X * ROOM_COUNT_Y);
    mark_room_visited();

    // Since -1 is not possible. This forces
    // redraw
    oldKeys = 255;
    oldGems = 255;
    oldBatteryLevel = 255;

    transitionTimer = 0;
    transitionMode = 2;
    
    quit = false;

    return false;
}


static bool quit_event() {

    if (init_title_screen_scene()) {

        return true;
    }

    core_register_callbacks(title_screen_refresh, 
        title_screen_redraw);

    return false;
}


static bool go_to_ending() {

    if (init_ending_scene()) {

        return true;
    }

    core_register_callbacks(
        ending_refresh,
        ending_redraw);

    return false;
}


bool game_refresh(i16 step) {   

    // Hard-coding woohoo
    const u8 REQUIRED_PLACED_GEMS = 1;

    if (quit) {
        
        return quit_event();
    }

    if (transitionMode != 0) {

        if ((transitionTimer -= step) <= 0) {

            if ((-- transitionMode) > 0) {

                if (transitionMode == 2) 
                    player->startPos = stage_find_player(gameStage);

                // TODO: Callback function?
                pl_reset(player, gameStage);
                stage_reset_room(gameStage);
                stage_flush_redraw_buffer(gameStage);

                if (transitionMode == 2) {

                    mapDrawn = false;
                    stage_recompute_wall_data(gameStage);
                    transitionMode = 1;
                }

                transitionTimer += TRANSITION_TIME;       
            }
        }
        return false;
    }

    if (msgBox->active) {

        if (msg_update(msgBox, step)) {

            if (gameStage->gemsPlaced >= REQUIRED_PLACED_GEMS) {

                return go_to_ending();
            }

            stage_redraw_all(gameStage);
        }
        return false;
    }

    // Trigger ending
    if (gameStage->gemsPlaced >= REQUIRED_PLACED_GEMS) {

        return msg_build(msgBox, ENDING);
    }

    if (pauseMenu->active) {

        menu_update(pauseMenu, step);
        return false;
    }
    else if (keyb_get_normal_key(KEY_RETURN) == STATE_PRESSED) {

        menu_activate(pauseMenu, 0);
        return false;
    }

    if (keyb_get_normal_key(KEY_R) == STATE_PRESSED) {

        transitionTimer = TRANSITION_TIME;
        transitionMode = 2;
        return false;
    }

    if (pl_update(player, gameStage, msgBox, step)) {

        mark_room_visited();
        mapDrawn = false;
    }
    if (stage_update(gameStage, step)) {

        pl_force_wait(player, gameStage);
    }

    return false;
}


static void draw_map_frame(i16 x, i16 y) {

    draw_frame(bmpHUD, x, y, MAP_WIDTH, MAP_HEIGHT, 6, 1, false);
    fill_rect(x, y, MAP_WIDTH, MAP_HEIGHT, 0);

    draw_text_fast(bmpFont, "MAP:", 80-11, y-15, -1, true);
}


static void draw_map(i16 x, i16 y) {

    const i16 ROOM_WIDTH = 3;
    const i16 ROOM_HEIGHT = 10;

    i16 dx, dy;
    i16 sy;

    i16 cx = gameStage->camPos.x / gameStage->roomWidth;
    i16 cy = gameStage->camPos.y / gameStage->roomHeight;

    for (dy = 0; dy < ROOM_COUNT_Y; ++ dy) {

        for (dx = 0; dx < ROOM_COUNT_X; ++ dx) {

            if (!visitedRooms[dy * ROOM_COUNT_X + dx])
                continue;

            if (dx == cx && dy == cy) {

                sy = 0;
            }
            else {

                sy = ROOM_HEIGHT;
            }

            draw_bitmap_region_fast(bmpHUD,
                12, sy, ROOM_WIDTH, ROOM_HEIGHT,
                x + dx * ROOM_WIDTH, 
                y + dy * ROOM_HEIGHT);
        }
    }
}


static void draw_treasure(i16 x, i16 y) {

    char buffer[8];

    if (oldKeys != player->keys) {

        // Keys
        snprintf(buffer, 8, "\2%d", player->keys);
        draw_bitmap_region_fast(bmpHUD, 0, 24, 4, 16, x, y);
        draw_text_fast(bmpFont, buffer, x+4, y+4, -1, false);

        oldKeys = player->keys;
    }

    if (oldGems != player->gems) {

        // Gems
        snprintf(buffer, 8, "\2%d", player->gems);
        draw_bitmap_region_fast(bmpHUD, 4, 24, 4, 16, x+10, y);
        draw_text_fast(bmpFont, buffer, x+14, y+4, -1, false);

        oldGems = player->gems;
    }
}


static void draw_battery(i16 x, i16 y) {

    i16 i;
    i16 sx;

    draw_bitmap_region_fast(bmpHUD, 8, 24, 4, 16, x, y);

    for (i = 0; i < PLAYER_MAX_BATTERY_LEVEL; ++ i) {

        sx = (i16)(i < (i16)player->battery) * 2;

        draw_bitmap_region_fast(bmpHUD, sx + 12, 24, 2, 16, x + 4+i, y);
    }
}


static void draw_hud_static() {

    clear_screen(1);

    // Needed for the initial black bar effect
    fill_rect(gameStage->xoff, 
        gameStage->yoff,
        gameStage->roomWidth * 4, 
        gameStage->roomHeight*16, 0);

    draw_frame(bmpHUD, gameStage->xoff, gameStage->yoff,
        gameStage->roomWidth*4,
        gameStage->roomHeight*16, 
        0, 0, true);

        
    // Logo
    draw_bitmap_fast(bmpLogo, 80 - 21, 8);

    // Map
    draw_map_frame(MAP_X, MAP_Y);

    // Treasure & items
    draw_text_fast(bmpFont, "TREASURE:", 80-11, 134, -1, true);

    // Battery
    draw_text_fast(bmpFont, "BATTERY:", 80-11, 172-8, -1, true);
}


static void draw_hud_changing() {

    if (!mapDrawn) {

        draw_map(MAP_X, MAP_Y);
        mapDrawn = true;
    }

    // Treasure
    draw_treasure(80-21, 143);

    if (oldBatteryLevel != player->battery) {

        // Battery
        draw_battery(80-20, 174);

        oldBatteryLevel = player->battery;
    }
}


static void set_clipping_area() {

    toggle_clipping(true);
    set_clip_rectangle(
        gameStage->xoff, 
        gameStage->yoff,
        gameStage->roomWidth * 4, 
        gameStage->roomHeight*16);
}


// Not really a transition, but oh well
static void draw_transition() {

    i16 x = gameStage->xoff;
    i16 y = gameStage->yoff;

    i16 w = gameStage->roomWidth * 4;
    i16 h = gameStage->roomHeight * 16;

    i16 trHeight;
    i16 stageRow;

    if (transitionMode >= 2) {

        trHeight = ((h/2 + 16) << 4) / TRANSITION_TIME * (TRANSITION_TIME - transitionTimer);
        trHeight >>= 8;
        trHeight <<= 4;
        
        fill_rect(x, y, w, trHeight, 0);
        fill_rect(x, y + h - trHeight, w, trHeight, 0);
    }
    else {

        oldTrHeight = trHeight;
        trHeight = ((h/2 + 32) << 4) / TRANSITION_TIME * transitionTimer;
        trHeight >>= 4;

        stageRow = -2 + fixed_round(trHeight, 16);

        if (stageRow < gameStage->roomHeight/2 &&
            oldTrHeight / 16 != stageRow) {

            stage_partial_redraw(gameStage, stageRow );
            stage_partial_redraw(gameStage, gameStage->roomHeight-1 - stageRow );

            stage_draw(gameStage, bmpTileset);
            stage_draw_objects(gameStage, bmpObjects);
        }
        
        if ((i16)abs(player->pos.y - gameStage->roomHeight/2) < 5 - stageRow) {

            set_clipping_area();
            pl_draw(player, bmpFigure);
            toggle_clipping(false);
        }
        
    }
}


void game_redraw() {

    if (quit) return;

    if (transitionMode == 0 && msgBox->active) {

        msg_draw(msgBox, bmpFont,
            gameStage->xoff, gameStage->yoff,
            gameStage->roomWidth*4, 
            gameStage->roomHeight*16);
        return;
    }

    if (transitionMode == 0 && pauseMenu->active) {

        menu_draw(pauseMenu, bmpFont, 
            gameStage->xoff + gameStage->roomWidth*2,
            gameStage->yoff + gameStage->roomHeight*8);
        return;
    }

    if (!bgDrawn) {

        draw_hud_static();
        bgDrawn = true;
    }

    draw_hud_changing();

    if (transitionMode != 0) {

        draw_transition();
        return;
    }

    pl_update_stage_tile_buffer(player, gameStage);
    stage_draw(gameStage, bmpTileset);
    stage_draw_objects(gameStage, bmpObjects);
    stage_draw_effects(gameStage, bmpTileset);

    set_clipping_area();
    pl_draw(player, bmpFigure);
    toggle_clipping(false);
}


void dispose_game_scene() {

    dispose_bitmap(bmpFont);
    dispose_bitmap(bmpFigure);
    dispose_bitmap(bmpTileset);
    dispose_bitmap(bmpHUD);
    dispose_bitmap(bmpObjects);
    dispose_bitmap(bmpLogo);

    dispose_tilemap(baseMap);
    
    dispose_stage(gameStage);

    if (player != NULL)
        free(player);

    if (visitedRooms != NULL)
        free(visitedRooms);

    if (msgBox != NULL)
        free(msgBox);

    if (pauseMenu != NULL)
        free(pauseMenu);
}
