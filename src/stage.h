#ifndef __STAGE__
#define __STAGE__

#include "tilemap.h"
#include "bitmap.h"


typedef struct {

    Tilemap* baseMap;
    i16 roomWidth;
    i16 roomHeight;
    i16 yoff;
    Vector2 camPos;

    u8* renderBuffer;

} Stage;


Stage* new_stage(Tilemap* baseMap, 
    i16 roomWidth, i16 roomHeight, i16 camX, i16 camY);
void dispose_stage(Stage* s);

void stage_draw(Stage* s, Bitmap* bmpTileset);

#endif // __STAGE__
