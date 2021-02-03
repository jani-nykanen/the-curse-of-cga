#include "player.h"

#include "graph.h"
#include "keyb.h"
#include "util.h"


#define MOVE_TIME 20
static const i16 INTERACTION_TIME = MOVE_TIME + 1;


Player create_player(i16 x, i16 y, Stage* s) {

    Player pl;
    pl.pos = vec2(x, y);
    pl.target = pl.pos;
    pl.rpos = vec2(s->xoff + pl.pos.x * 4, 
        s->yoff + pl.pos.y*16);

    pl.moveTimer = 0;
    pl.moving = false;
    pl.interacting = false;
    pl.forcedInteraction = false;
    pl.dir = vec2(0, 0);

    pl.spr = create_sprite(4, 16);

    pl.keys = 0;
    pl.gems = 0;
    pl.battery = 0;

    return pl;
}


static void pl_compute_render_pos(Player* pl, Stage* s) {

    pl->rpos = vec2(s->xoff + pl->pos.x * 4, s->yoff + pl->pos.y*16);
}


static void pl_control(Player* pl, Stage* s, i16 step) {

    i16 dx = 0;
    i16 dy = 0;
    State k;
    u8 actionType;

    if (pl->moving || pl->interacting) return;

    if ((k = keyb_get_ext_key(KEY_RIGHT)) & STATE_DOWN_OR_PRESSED) {

        pl->spr.row = 1;
        dx = 1;
    }
    else if ((k = keyb_get_ext_key(KEY_LEFT)) & STATE_DOWN_OR_PRESSED) {

        pl->spr.row = 3;
        dx = -1;
    }
    else if ((k = keyb_get_ext_key(KEY_DOWN)) & STATE_DOWN_OR_PRESSED) {

        pl->spr.row = 0;
        dy = 1;
    }
    else if ((k = keyb_get_ext_key(KEY_UP)) & STATE_DOWN_OR_PRESSED) {

        pl->spr.row = 2;
        dy = -1;
    }

    actionType = 0;
    if (dx != 0 || dy != 0) {

        actionType = stage_movement_collision(s, k,
                pl->pos.x + dx, pl->pos.y + dy, 
                dx, dy, MOVE_TIME, 
                &pl->battery, &pl->keys, &pl->gems);
    }

    if (actionType == 1) {

        pl->target.x = pl->pos.x + dx;
        pl->target.y = pl->pos.y + dy;

        pl->dir.x = pl->target.x - pl->pos.x;
        pl->dir.y = pl->target.y - pl->pos.y;

        pl->moveTimer = MOVE_TIME;
        pl->moving = true;
        pl->forcedInteraction = false;
    }
    else if (actionType == 2) {

        pl->moveTimer = INTERACTION_TIME;
        pl->interacting = true;
        pl->forcedInteraction = false;
        pl->moving = false;
    }
}


static bool pl_check_camera(Player* pl, Stage* s) {

    i16 dx, dy;

    if (stage_check_camera_transition(s,
        pl->target.x, pl->target.y)) {

        dx = (pl->target.x - pl->pos.x) * (s->roomWidth);
        dy = (pl->target.y - pl->pos.y) * (s->roomHeight); 

        pl->pos.x -= dx;
        pl->pos.y -= dy;    

        pl->target.x -= dx;
        pl->target.y -= dy;

        pl_compute_render_pos(pl, s);

        // Reset battery
        pl->battery = 0;

        return true;
    }
    return false;
}


static bool pl_animate_interaction(Player* pl, i16 step) {

    if (pl->interacting) {

        if ((pl->moveTimer -= step) <= 0) {

            pl->interacting = false;
            pl->forcedInteraction = false;
            spr_set_frame(&pl->spr, 0, pl->spr.row);
        }
        else {

            spr_set_frame(&pl->spr, 
                pl->forcedInteraction ? 0 : 4, 
                pl->spr.row);
        }
    }
    return pl->interacting;
}


static bool pl_tile_check(Player* pl, Stage* s) {

    u8 res = stage_check_overlay(s, pl->pos.x, pl->pos.y);
    
    if (res >= 1) {

        switch (res) {

        case 1:

            ++ pl->keys;
            break;

        case 2:

            pl->battery = min_i16(PLAYER_MAX_BATTERY_LEVEL, pl->battery + 3);
            break;

        case 3:

            ++ pl->gems;
            break;

        default:
            break;
        }
    }
    else {

        if (stage_check_automatic_movement(s, pl->pos.x, pl->pos.y, &pl->target) == 1) {

            pl->dir.x = pl->target.x - pl->pos.x;
            pl->dir.y = pl->target.y - pl->pos.y;

            pl->moveTimer = MOVE_TIME;
            pl->moving = true;

            pl->forcedInteraction = true;

            if (stage_check_conflict(s, pl->target.x, pl->target.y)) {

                pl->target = pl->pos;
                pl->moving = false;
                return false;
            }
        }
    }

    return false;
}


static bool pl_move(Player* pl, Stage* s, i16 step) {

    i16 moveStep = MOVE_TIME / 4;
    i16 delta;
    i16 oldTime;
    bool ret = false;
    
    if (!pl->moving || pl->interacting) return false;

    oldTime = pl->moveTimer;
    if ((pl->moveTimer -= step) <= 0) {

        pl->moving = false;
        pl->pos = pl->target;
        pl_tile_check(pl, s);

        pl_compute_render_pos(pl, s);

        return false;
    }

    if (oldTime >= MOVE_TIME/2 && pl->moveTimer < MOVE_TIME/2) {

        ret = pl_check_camera(pl, s);
    }

    delta = 4 - fixed_round(pl->moveTimer, moveStep);
    pl->rpos.x += pl->dir.x * delta;
    pl->rpos.y += pl->dir.y * delta * 4;

    return ret;
}


static void pl_animate(Player* pl, i16 step) {

    const i16 ANIM_SPEED = 8;


    if (pl->interacting) {

        pl_animate_interaction(pl, step);
        return;
    }

    if (pl->moving && !pl->forcedInteraction) {

        spr_animate(&pl->spr, pl->spr.row, 0, 3, ANIM_SPEED, step);
    }
    else {

        spr_set_frame(&pl->spr, 0, pl->spr.row);
    }
}



bool pl_update(Player* pl, Stage* s, i16 step) {
    
    bool ret = false;

    pl_control(pl, s, step);
    pl_compute_render_pos(pl, s);
    pl_animate(pl, step);
    ret = pl_move(pl, s, step);

    stage_mark_tile_solid(s, pl->pos.x, pl->pos.y, false);
    stage_mark_tile_solid(s, pl->target.x, pl->target.y, true);
    
    return ret;
}


void pl_draw(Player* pl, Bitmap* bmpPlayer) {

    const i16 Y_OFF = -2;

    draw_sprite(&pl->spr, bmpPlayer, pl->rpos.x, pl->rpos.y + Y_OFF);
}


void pl_update_stage_tile_buffer(Player* pl, Stage* s) {

    i16 i;

    for (i = 0; i < 2; ++ i) {

        stage_mark_tile_for_redraw(s, 
            pl->pos.x % s->roomWidth, 
            (pl->pos.y-i) % s->roomHeight);

        stage_mark_tile_for_redraw(s, 
            pl->target.x % s->roomWidth, 
            (pl->target.y-i) % s->roomHeight);
    }
}


void pl_force_wait(Player* pl, Stage* s) {

    pl->pos = pl->target;
    pl->moving = false;
    pl_compute_render_pos(pl, s);

    pl->interacting = true;
    pl->forcedInteraction = true;
    pl->moveTimer = INTERACTION_TIME;

}
