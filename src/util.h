#ifndef __UTILITY__
#define __UTILITY__

#include "types.h"


#define alloc_object(target, T, ret) target = (T*) malloc(sizeof(T));\
    if (target == NULL) {\
        err_throw("Memory allocation error!"); \
        return ret;\
    }


i16 fixed_round(i16 x, i16 d);


#endif // __UTILITY__
