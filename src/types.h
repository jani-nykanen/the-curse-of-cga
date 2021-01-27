#ifndef __TYPES__
#define __TYPES__

#include <stdbool.h>

typedef char byte;
typedef signed char i8;
typedef char u8;

typedef signed short i16;
typedef unsigned short u16;

typedef unsigned long ulong;

// On 16-bit compilers this *should*
// be right
typedef signed long i32;
typedef unsigned long u32;

typedef char* str;

typedef struct {

    i16 x;
    i16 y;

} Vector2;

Vector2 vec2(i16 x, i16 y);

#endif // __TYPES__
