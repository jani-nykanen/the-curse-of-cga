#ifndef __TYPES__
#define __TYPES__

#include <stdbool.h>

typedef char byte;
typedef signed char i8;
typedef char u8;

typedef signed short i16;
typedef unsigned short u16;

// It should be noted that on 32-bit compiler
// this is not true
typedef signed long i32;
typedef unsigned long u32;

typedef char* str;

typedef struct {

    i32 x;
    i32 y;

} Vector2;

Vector2 vec2(i32 x, i32 y);

#endif // __TYPES__
