#include "tilemap.h"
#include "err.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


Tilemap* load_tilemap(const str path) {


    Tilemap* tmap;
    FILE* f;
    u16 i;

    f = fopen(path, "rb");
    if (f == NULL) {

        err_throw_str("Failed to open a file in ", (str)path);
        return NULL;
    }

    tmap = (Tilemap*)malloc(sizeof(Tilemap));
    if (tmap == NULL) {

        ERR_MALLOC();
        fclose(f);
        return NULL;
    }

    fread(&tmap->width, sizeof(u16), 1, f);
    fread(&tmap->height, sizeof(u16), 1, f);
    fread(&tmap->layerCount, sizeof(u16), 1, f);

    tmap->data = (u8**)malloc(sizeof(u8*) * tmap->layerCount);
    if (tmap->data == NULL) {

        ERR_MALLOC();
        dispose_tilemap(tmap);
        fclose(f);
        return NULL;
    }

    for (i = 0; i < tmap->layerCount; ++ i) {

        tmap->data[i] = (u8*)malloc(sizeof(u8) * tmap->width * tmap->height);
        if (tmap->data[i] == NULL) {

            ERR_MALLOC();
            dispose_tilemap(tmap);
            fclose(f);
            return NULL;
        }   
        fread(tmap->data[i], 1, tmap->width * tmap->height, f);
    }

    fclose(f);

    return tmap;
}


void dispose_tilemap(Tilemap* tmap) {

    u16 i;

    if (tmap == NULL) return;

    if (tmap->data != NULL) {

        for (i = 0; i < tmap->layerCount; ++ i) {

            if (tmap->data[i] != NULL)
                free(tmap->data[i]);
        }
        free(tmap->data);
    }

    free(tmap);
}


u8 tmap_get_tile(Tilemap* tmap, u16 layer, i16 x, i16 y, u8 def) {

    if (layer >= tmap->layerCount || 
        x < 0 || x >= tmap->width ||
        y < 0 || y >= tmap->height) 
        return def;

    return tmap->data[layer][y * tmap->width + x];
}


void tmap_set_tile(Tilemap* tmap, u16 layer, i16 x, i16 y, u8 v) {

    if (layer >= tmap->layerCount || 
        x < 0 || x >= tmap->width ||
        y < 0 || y >= tmap->height) 
        return;

    tmap->data[layer][y * tmap->width + x] = v;
}


void tmap_clone_area(Tilemap* tmap, u8* out,
    u16 layer, i16 x, i16 y, i16 w, i16 h) {

    i16 dy;

    u32 djump = 0;
    u32 sjump = y * tmap->width + x;

    for (dy = 0; dy < h; ++ dy) {

        memcpy(out + djump, tmap->data[layer] + sjump, w);
        djump += w;
        sjump += tmap->width;
    }
}
