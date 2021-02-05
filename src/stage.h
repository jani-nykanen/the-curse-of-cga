#ifndef __STAGE__
#define __STAGE__

#include "tilemap.h"
#include "bitmap.h"
#include "keyb.h"


typedef struct {

    i16 srcx[4];
    i16 srcy[4];

} TileWallData;


typedef struct {

    Vector2 pos;
    Vector2 target;
    Vector2 rpos;
    i32 timer;
    i32 startTime;

} MovingRock;


typedef struct {

    Tilemap* baseMap;
    i16 roomWidth;
    i16 roomHeight;
    i16 xoff;
    i16 yoff;
    Vector2 camPos;
    Vector2 initialCamPos;

    u8* renderBuffer;
    TileWallData* wallTiles;
    u8* roomTilesStatic;
    u8* roomTilesDynamic;

    MovingRock* rockAnim;
    Vector2 disappearPos;
    i16 disappearTimer;
    u8 disappearTile;

} Stage;


Stage* new_stage(Tilemap* baseMap, 
    i16 roomWidth, i16 roomHeight, Vector2* startPos);
void dispose_stage(Stage* s);

bool stage_update(Stage* s, i16 step);

void stage_draw(Stage* s, Bitmap* bmpTileset);
void stage_draw_objects(Stage* s, Bitmap* bmpObjects);
void stage_draw_effects(Stage* s, Bitmap* bmpTileset);

void stage_mark_tile_for_redraw(Stage* s, i16 x, i16 y);
void stage_redraw_all(Stage* s);

u8 stage_movement_collision(Stage* s, State actionType,
    i16 x, i16 y, i16 dx, i16 dy, i16 objectMoveTime,
    u8* access, u8* interactionLevel, u8* keyCount, u8* gemCount);

u8 stage_check_automatic_movement(Stage* s, i16 x, i16 y, Vector2* target);

bool stage_check_camera_transition(Stage* s, i16 x, i16 y);
u8 stage_check_overlay(Stage* s, i16 x, i16 y);
bool stage_check_conflict(Stage* s, i16 x, i16 y);

void stage_mark_tile_solid(Stage* s, i16 x, i16 y, bool state);

void stage_reset_room(Stage* s);

void stage_flush_redraw_buffer(Stage* s);
void stage_partial_redraw(Stage* s, i16 row);

Vector2 stage_find_player(Stage* s);
void stage_recompute_wall_data(Stage* s);

#endif // __STAGE__
