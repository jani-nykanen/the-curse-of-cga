#ifndef __TILEMAP__
#define __TILEMAP__


#include "types.h"


typedef struct {

    u8** data;
    u16 layerCount;
    u16 width;
    u16 height;

} Tilemap;


Tilemap* load_tilemap(const str path);
void dispose_tilemap(Tilemap* tmap);

i16 tmap_get_tile(Tilemap* tmap, u16 layer, i16 x, i16 y, i16 def);

void tmap_clone_area_i16(Tilemap* tmap, i16* out,
    u16 layer, i16 x, i16 y, i16 w, i16 h);

#endif // __TILEMAP__
