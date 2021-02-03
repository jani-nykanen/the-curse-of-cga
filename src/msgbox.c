#include "msgbox.h"
#include "err.h"
#include "graph.h"
#include "keyb.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static const i16 CHAR_TIME = 4;


static void compute_max_dimensions(const str msg, u16* w, u16* h) {

    u16 maxWidth = 0;
    u16 height = 1;
    u16 len = 0;
    u16 i;

    for (i = 0; i < strlen(msg); ++ i) {

        if (msg[i] == '\n') {

            if (len > 0 && len-1 > maxWidth)
                maxWidth = len-1;
            len = 0;

            ++ height;
        }
        ++ len;
    }

    if (len > 0 && len-1 > maxWidth)
        maxWidth = len-1;

    *w = maxWidth;
    *h = height;
}


MessageBox create_message_box() {

    MessageBox m;

    m.buffer = NULL;
    m.active = false;

    return m;
}


bool msg_build(MessageBox* box, const str msg) {

    if (box->buffer != NULL)
        free(box->buffer);

    box->buffer = (str) malloc(strlen(msg));
    if (box->buffer == NULL) {

        ERR_MALLOC();
        return true;
    }
    box->bufferLen = strlen(msg);
    strcpy(box->buffer, msg);

    box->charPointer = 0;
    box->lastCharPointer = 0;
    box->charTimer = 0;
    box->active = true;
    box->boxDrawn = false;

    compute_max_dimensions(msg, &box->width, &box->height);

    return false;
}


bool msg_update(MessageBox* box, i16 step){

    if (!box->active) return false;

    if (box->charPointer <= box->bufferLen) {

        if (box->charPointer > 0 && keyb_any_pressed()) {

            box->charPointer = box->bufferLen + 1;
        }
        else {

            if ((box->charTimer += step) >= CHAR_TIME) {

                box->charTimer -= CHAR_TIME;
                ++ box->charPointer;
            }
        }
    }
    else if (keyb_any_pressed()) {

        box->active = false;
        return true;
    }

    return false;
}


void msg_draw(MessageBox* box, Bitmap* bmpFont,
    i16 x, i16 y, i16 dw, i16 dh) {

    const i16 SHADOW_OFFSET = 4;
    const i16 TEXT_OFFSET = 4;

    i16 dx, dy, w, h;

    if (!box->active) return;

    w = box->width * 2 + TEXT_OFFSET/2;
    h = box->height * 8 + TEXT_OFFSET*2;

    dx = x + dw/2 - w/2;
    dy = y + dh/2 - h/2;

    if (!box->boxDrawn) {

        fill_rect(dx + SHADOW_OFFSET/4, 
            dy + SHADOW_OFFSET, 
            w, h, 0);
        fill_rect(dx, dy, w, h, 1);

        box->boxDrawn = true;
    }

    draw_text_fast(bmpFont, (const str)box->buffer, 
        dx + TEXT_OFFSET/4, dy + TEXT_OFFSET,
        box->lastCharPointer, false);
    box->lastCharPointer = box->charPointer;

}


void msg_free_buffer(MessageBox* box) {

    if (box->buffer == NULL)
        return;

    free(box->buffer);
}
