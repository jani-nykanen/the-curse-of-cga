#ifndef __PLAYER__
#define __PLAYER__

#include "bitmap.h"
#include "sprite.h"
#include "stage.h"

#define PLAYER_MAX_BATTERY_LEVEL 12

typedef struct {

    Vector2 pos;
    Vector2 target;
    Vector2 rpos;
    Vector2 dir;
    
    bool moving;
    bool interacting; // Couldn't come up with a better word sorry
    bool forcedInteraction;
    i16 moveTimer;

    Sprite spr;

    // Item amounts
    u8 keys;
    u8 gems;
    u8 battery;

} Player;


Player create_player(i16 x, i16 y, Stage* s);

bool pl_update(Player* pl, Stage* s, i16 step);
void pl_draw(Player* pl, Bitmap* bmpPlayer);

void pl_update_stage_tile_buffer(Player* pl, Stage* s);

void pl_force_wait(Player* pl, Stage* s);

#endif // __PLAYER__
