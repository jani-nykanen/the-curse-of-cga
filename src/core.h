#ifndef __CORE__
#define __CORE__

#include "types.h"

typedef bool (*CoreCallbackRefresh) (i16);
typedef void (*CoreCallbackRedraw) (void);

void init_core();

void core_register_callbacks(
    CoreCallbackRefresh refresh,
    CoreCallbackRedraw redraw);

void core_run(i16 frameSkip);
void core_dispose();

#endif // __CORE__
