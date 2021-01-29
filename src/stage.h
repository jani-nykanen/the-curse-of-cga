#ifndef __STAGE__
#define __STAGE__

#include "tilemap.h"
#include "bitmap.h"


typedef struct {

    i16 srcx[4];
    i16 srcy[4];

} TileWallData;


typedef struct {

    Tilemap* baseMap;
    i16 roomWidth;
    i16 roomHeight;
    i16 xoff;
    i16 yoff;
    Vector2 camPos;

    u8* renderBuffer;
    TileWallData* wallTiles;
    i16* roomTiles;

} Stage;


Stage* new_stage(Tilemap* baseMap, 
    i16 roomWidth, i16 roomHeight, i16 camX, i16 camY);
void dispose_stage(Stage* s);

void stage_draw(Stage* s, Bitmap* bmpTileset);
void stage_draw_objects(Stage* s, Bitmap* bmpObjects);

void stage_mark_tile_for_redraw(Stage* s, i16 x, i16 y);

bool stage_is_tile_solid(Stage* s, i16 x, i16 y);
bool stage_check_camera_transition(Stage* s, i16 x, i16 y);

#endif // __STAGE__
