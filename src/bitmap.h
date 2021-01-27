#ifndef __BITMAP__
#define __BITMAP__

#include "types.h"


typedef struct {

    byte* pixels;
    byte* mask;

    u16 width;
    u16 height;

} Bitmap;


Bitmap* load_bitmap(const str path);
void dispose_bitmap(Bitmap* bmp);


#endif // __BITMAP__

