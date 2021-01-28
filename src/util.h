#ifndef __UTILITY__
#define __UTILITY__


#define alloc_object(target, T, ret) target = (T*) malloc(sizeof(T));\
    if (target == NULL) {\
        err_throw("Memory allocation error!"); \
        return ret;\
    }


#endif // __UTILITY__
