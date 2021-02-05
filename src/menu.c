#include "menu.h"

#include "err.h"
#include "util.h"
#include "keyb.h"
#include "graph.h"

#include <stdlib.h>
#include <string.h>


Menu* new_menu(const str* buttonNames, MenuCallback buttonCb, u16 len) {

    i16 i;
    Menu* m = (Menu*)malloc(sizeof(Menu));
    if (m == NULL) {

        ERR_MALLOC();
        return NULL;
    }

    m->buttonNames = (str*)calloc(len, sizeof(str));
    if (m->buttonNames == NULL) {

        dispose_menu(m);
        return NULL;
    }

    m->width = 0;
    m->height = (i16)len;
    for (i = 0; i < len; ++ i) {

        m->buttonNames[i] = (str)malloc(sizeof(char) * strlen(buttonNames[i]));
        if (m->buttonNames[i] == NULL) {

            dispose_menu(m);
            return NULL;
        }
        strcpy(m->buttonNames[i], buttonNames[i]);

        if (strlen(buttonNames[i]) > m->width) {

            m->width = (i16)strlen(buttonNames[i]);
        }
    }

    m->buttonCb = buttonCb;
    m->active = false;
    m->buttonCount = len;
    m->boxDrawn = false;
    m->cursorPos = 0;

    return m;
}


void dispose_menu(Menu* m) {

    i16 i;

    // TODO: Not every component is NULL by default,
    // so if something fails, this attempts to free memory
    // that is not NULL, but neither is it reserved

    if (m == NULL) return;

    if (m->buttonNames != NULL) {

        for (i = 0; i < m->buttonCount; ++ i) {

            if (m->buttonNames[i] != NULL)
                free(m->buttonNames[i]);
        }
        free(m->buttonNames);
    }

    free(m);
}


void menu_update(Menu* m, i16 step) {

    i16 oldPos = m->cursorPos;

    if (!m->active) return;

    if (keyb_get_ext_key(KEY_DOWN) == STATE_PRESSED) {

        ++ m->cursorPos;
    }
    else if (keyb_get_ext_key(KEY_UP) == STATE_PRESSED) {

        -- m->cursorPos;
    }
    m->cursorPos = neg_mod(m->cursorPos, (i16)m->buttonCount);

    if (m->cursorPos != oldPos) {

        // Play a sound effect
    }

    if (keyb_get_normal_key(KEY_RETURN) == STATE_PRESSED) {

        m->buttonCb(m->cursorPos);
    }
}


void menu_draw(Menu* m, Bitmap* bmpFont, i16 dx, i16 dy) {

    i16 x, y, w, h;
    i16 i;

    if (!m->active) return;

    w = (m->width + 3) * 2;
    h = (m->height + 1) * 10;

    x = dx - w/2;
    y = dy - h/2;

    if (!m->boxDrawn) {

        draw_box(x, y, w, h, 4);

        // Also draw buttons here because why not
        for (i = 0; i < m->buttonCount; ++ i) {

            draw_text_fast(bmpFont, (const str)m->buttonNames[i], x + 5, y + 5 + i*10, -1, false);
        }

        m->boxDrawn = true;
    }

    fill_rect(x+1, y+5, 4, m->buttonCount*10, 1);
    draw_bitmap_region_fast(bmpFont, 
        6, 0, 4, 8, 
        x + 1, 
        y + 5 + m->cursorPos*10);
        
}


void menu_activate(Menu* m, i16 cursorPos) {

    m->active = true;
    m->cursorPos = cursorPos % m->buttonCount;
    m->boxDrawn = false;
}
