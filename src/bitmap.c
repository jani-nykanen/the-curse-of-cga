#include "bitmap.h"

#include "err.h"

#include <stdio.h>
#include <stdlib.h>


Bitmap* load_bitmap(const str path) {

    Bitmap* bmp;
    FILE* f;
    byte hasMask;

    f = fopen(path, "rb");
    if (f == NULL) {

        err_throw_str("Failed to open a file in ", (str)path);
        return NULL;
    }

    bmp = (Bitmap*)malloc(sizeof(Bitmap));
    if (bmp == NULL) {

        ERR_MALLOC();
        fclose(f);
        return NULL;
    }

    fread(&bmp->width, sizeof(u16), 1, f);
    fread(&bmp->height, sizeof(u16), 1, f);
    fread(&hasMask, 1, 1, f);

    bmp->pixels = (byte*) malloc(bmp->width * bmp->height / 4);
    if (bmp->pixels == NULL) {

        free(bmp);

        ERR_MALLOC();
        fclose(f);
        return NULL;
    }

    fread(bmp->pixels, 1, bmp->width * bmp->height / 4, f);

    if (hasMask) {

        bmp->mask = (byte*) malloc(bmp->width * bmp->height / 4);
        if (bmp->mask == NULL) {

            free(bmp->pixels);
            free(bmp);

            ERR_MALLOC();
            fclose(f);
            return NULL;
        }
        fread(bmp->mask, 1, bmp->width * bmp->height / 4, f);
    }
    else {

        bmp->mask = NULL;
    }

    fclose(f);
    return bmp;
}   


void dispose_bitmap(Bitmap* bmp) {

    if (bmp == NULL) return;

    if (bmp->mask != NULL) {

        free(bmp->mask);
    }

    free(bmp->pixels);
    free(bmp);
}
