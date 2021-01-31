#include "util.h"


i16 fixed_round(i16 x, i16 d) {

    if (x % d <= d / 2)
        return x / d;
    return (x / d) + 1;
}


i16 min_i16(i16 a, i16 b) {

    return a < b ? a : b;
}


i16 max_i16(i16 a, i16 b) {

    return a > b ? a : b;
}


i16 neg_mod(i16 m, i16 n) {

    return (m % n + n) % n;
}


i16 ceil_i16(i16 x, i16 d) {
    
    return 1 + ((x-1) / d);
}


i16 round_i16(i16 x, i16 d) {

    if (x % d <= d / 2)
        return x / d;
    return (x / d) + 1;
}


i16 clamp_i16(i16 x, i16 min, i16 max) {

    return min_i16(max, max_i16(x, min));
}