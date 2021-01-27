/**
 * A tool for convering png images to binary, possibly
 * RLE-ncoded binary files with rgb332 palette
 *
 * (c) 2020 Jani Nykänen
 */

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <stdbool.h>


#define INTEGER(type, bits) typedef type i##bits; \
typedef unsigned type u##bits;

typedef unsigned char byte;

INTEGER(char, 8)
INTEGER(short, 16)
INTEGER(int, 32)
INTEGER(long, 64)

typedef float f32;
typedef double f64;

typedef char* str;

static unsigned char COLORS[][3] = {
    {0, 0, 0},
    {85, 255, 255},
    {255, 85, 255},
    {255, 255, 255}
};


static bool includes_str(const str word, str* argv, u32 len, u32 start) {
    
	u32 i;
	
	for (i = start; i < len; ++ i) {
		
		if (strcmp(word, argv[i]) == 0)
			return true;
	}
	return false;
}


static i32 find_color(u8* data, u32 loc) {

    i32 i, j;
    for (i = 0; i < 4; ++ i) {

        for (j = 0; j < 3; ++ j) {

            if (data[loc + j] != COLORS[i][j])
                break;

            if (j == 2)
                return i;
        }
    }
    return 0;
}


static void write_array(FILE* f, u8* arr, u32 len) {
    
    u32 i;
    u32 count = 3;
    u8 bout = 0;

    for (i = 0; i < len; ++ i) {

        bout = bout | (arr[i] << (count*2));

        if (count -- == 0) {

            fwrite(&bout, 1, 1, f);
            bout = 0;
            count = 3;
        }
    }
}


static i32 convert_bitmap(const str in, const str out, bool writeMask) {

    const u8 ALPHA_LIMIT = 127;

    i32 pixelCount;
    i32 w, h;
    i32 channels;
    u8* pdata;
    u8* cdata;
    u8* mask;

    u32 i;
    FILE* f;
    u8 bout;
    u16 sout;

    pdata = stbi_load(in, &w, &h, &channels, 4);
    if(pdata == NULL) {
        
        printf("Failed to load a bitmap in %s!\n", in);
        return 1;
    }
    pixelCount = w * h;

    cdata = (u8*)malloc(w * h);
    if (cdata == NULL) {

        printf("Memory allocation error!\n");
        return 1;
    }

    mask = (u8*)malloc(w * h);
    if (mask == NULL) {

        printf("Memory allocation error!\n");
        return 1;
    }

    for (i = 0; i < w * h; ++ i) {

        cdata[i] = find_color(pdata, i * 4);
        mask[i] = (u8)(pdata[i*4 + 3] >= ALPHA_LIMIT) * 3;
    }


    f = fopen(out, "wb");
    if (f == NULL) {

        printf("Failed to create a file to %s!\n", out);
        free(pdata);

        return 1;
    }

    // Dimensions
    sout = (u16)w;
    fwrite(&sout, sizeof(u16), 1, f);
    sout = (u16)h;
    fwrite(&sout, sizeof(u16), 1, f);

    // Does have a mask
    bout = (u8)writeMask;
    fwrite(&bout, 1, 1, f);

    // Pixel data
    write_array(f, cdata, w*h);
    // Mask data
    if (writeMask) {

        write_array(f, mask, w*h);
    }

    fclose(f);

    return 0;
}


// Main
i32 main(i32 argc, str* argv) {

    if(argc < 3) {

        printf("Must pass at least 2 arguments.\nHelp: png2cga input-path output-path (-mask)\n");
        return 1;
    }

    return convert_bitmap(argv[1], argv[2], includes_str("-mask", argv, argc, 1));
}
