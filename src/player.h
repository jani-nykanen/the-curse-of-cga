#ifndef __PLAYER__
#define __PLAYER__

#include "bitmap.h"
#include "sprite.h"
#include "stage.h"

typedef struct {

    Vector2 pos;
    Vector2 target;
    Vector2 rpos;
    Vector2 dir;
    
    bool moving;
    i16 moveTimer;

    Sprite spr;

} Player;


Player create_player(i16 x, i16 y, Stage* s);

void pl_update(Player* pl, Stage* s, i16 step);
void pl_draw(Player* pl, Bitmap* bmpPlayer);

void pl_update_stage_tile_buffer(Player* pl, Stage* s);

#endif // __PLAYER__
