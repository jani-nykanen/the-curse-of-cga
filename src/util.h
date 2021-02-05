#ifndef __UTILITY__
#define __UTILITY__

#include "types.h"


#define alloc_object(target, T, ret) target = (T*) malloc(sizeof(T));\
    if (target == NULL) {\
        err_throw("Memory allocation error!"); \
        return ret;\
    }


i16 fixed_round(i16 x, i16 d);

i16 min_i16(i16 a, i16 b);
i16 max_i16(i16 a, i16 b);

i16 neg_mod(i16 m, i16 n);

i16 clamp_i16(i16 x, i16 min, i16 max);

void draw_box(i16 dx, i16 dy, i16 w, i16 h, i16 shadowOff);

#endif // __UTILITY__
