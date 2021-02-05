#ifndef __MENU__
#define __MENU__

#include "types.h"

typedef void (*MenuCallback) (void);


typedef struct {

    str* buttonNames;
    MenuCallback* buttonCbs;
    u16 buttonCount;
    i16 cursorPos;
    bool active;

} Menu;


Menu new_menu(const str* buttonNames, MenuCallback* buttonCbs, u16 len);
void dispose_menu(Menu* menu);

void menu_update(Menu* m, i16 step);
void menu_draw(Menu* m);

void menu_activate(Menu* m, i16 cursorPos);

#endif // __MENU__
