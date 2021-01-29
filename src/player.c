#include "player.h"

#include "graph.h"
#include "keyb.h"
#include "util.h"


static const i16 MOVE_TIME = 20;


Player create_player(i16 x, i16 y, Stage* s) {

    Player pl;
    pl.pos = vec2(x, y);
    pl.target = pl.pos;
    pl.rpos = vec2(s->xoff + pl.pos.x * 4, 
        s->yoff + pl.pos.y*16);

    pl.moveTimer = 0;
    pl.moving = false;
    pl.dir = vec2(0, 0);

    pl.spr = create_sprite(4, 16);

    return pl;
}


static void pl_control(Player* pl, Stage* s, i16 step) {

    i16 dx = 0;
    i16 dy = 0;

    if (pl->moving) return;

    if (keyb_get_ext_key(KEY_RIGHT) & STATE_DOWN_OR_PRESSED) {

        pl->spr.row = 1;
        dx = 1;
    }
    else if (keyb_get_ext_key(KEY_LEFT) & STATE_DOWN_OR_PRESSED) {

        pl->spr.row = 3;
        dx = -1;
    }
    else if (keyb_get_ext_key(KEY_DOWN) & STATE_DOWN_OR_PRESSED) {

        pl->spr.row = 0;
        dy = 1;
    }
    else if (keyb_get_ext_key(KEY_UP) & STATE_DOWN_OR_PRESSED) {

        pl->spr.row = 2;
        dy = -1;
    }

    if ((dx != 0 || dy != 0) &&
        !stage_is_tile_solid(s, pl->pos.x + dx, pl->pos.y + dy)) {

        pl->target.x = pl->pos.x + dx;
        pl->target.y = pl->pos.y + dy;

        pl->dir.x = pl->target.x - pl->pos.x;
        pl->dir.y = pl->target.y - pl->pos.y;

        pl->moveTimer = MOVE_TIME;
        pl->moving = true;
    }
}


static void pl_move(Player* pl, Stage* s, i16 step) {

    i16 moveStep = MOVE_TIME / 4;
    i16 delta;
    
    if (!pl->moving) return;

    if ((pl->moveTimer -= step) <= 0) {

        pl->moving = false;
        pl->pos = pl->target;
        pl->rpos = vec2(s->xoff + pl->pos.x * 4, s->yoff + pl->pos.y*16);

        return;
    }

    delta = 4 - fixed_round(pl->moveTimer, moveStep);
    pl->rpos.x += pl->dir.x * delta;
    pl->rpos.y += pl->dir.y * delta * 4;

    
}


static void pl_animate(Player* pl, i16 step) {

    const i16 ANIM_SPEED = 8;

    if (pl->moving) {

        spr_animate(&pl->spr, pl->spr.row, 0, 3, ANIM_SPEED, step);
    }
    else {

        spr_set_frame(&pl->spr, 0, pl->spr.row);
    }
}


void pl_update(Player* pl, Stage* s, i16 step) {
    
    pl->rpos = vec2(s->xoff + pl->pos.x * 4, s->yoff + pl->pos.y*16);

    pl_control(pl, s, step);
    pl_animate(pl, step);
    pl_move(pl, s, step);
}


void pl_draw(Player* pl, Bitmap* bmpPlayer) {

    const i16 Y_OFF = -4;

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
