#include "util.h"


i16 fixed_round(i16 x, i16 d) {

    if (x % d <= d / 2)
        return x / d;
    return (x / d) + 1;
}
