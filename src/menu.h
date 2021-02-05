#ifndef __MENU__
#define __MENU__

#include "types.h"
#include "bitmap.h"

typedef void (*MenuCallback) (i16);


typedef struct {

    str* buttonNames;
    MenuCallback buttonCb;
    u16 buttonCount;
    i16 cursorPos;
    bool active;
    bool boxDrawn;
    i16 width;
    i16 height;

} Menu;


Menu* new_menu(const str* buttonNames, MenuCallback buttonCb, u16 len);
void dispose_menu(Menu* m);

void menu_update(Menu* m, i16 step);
void menu_draw(Menu* m, Bitmap* bmpFont, i16 dx, i16 dy);

void menu_activate(Menu* m, i16 cursorPos);

#endif // __MENU__
