#ifndef __KEYB__
#define __KEYB__

#include "keycodes.h"
#include "types.h"

typedef enum {

    STATE_UP = 0,
    STATE_RELEASED = 2,

    STATE_DOWN = 1,
    STATE_PRESSED = 3,
    
    // Used with '&' operator
    STATE_DOWN_OR_PRESSED = 1,

} State;

void init_keyboard_listener();
void reset_keyboard_listener();

void keyb_update();
bool keyb_any_pressed();

State keyb_get_normal_key(u8 key);
State keyb_get_ext_key(u8 key);

#endif // __KEYB__
