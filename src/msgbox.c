#include "msgbox.h"
#include "err.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static const i16 CHAR_TIME = 8;


static void compute_max_dimensions(const str msg, u16* w, u16* h) {

    u16 maxWidth = 0;
    u16 height = 1;
    u16 len = 0;
    u16 i;

    for (i = 0; i < strlen(msg); ++ i) {

        if (msg[i] == '\n') {

            if (len > maxWidth)
                maxWidth = len;
            len = 0;

            ++ height;
        }
        ++ len;
    }

    if (len > maxWidth)
        maxWidth = len;

    *w = maxWidth;
    *h = height;
}


MessageBox create_message_box() {

    MessageBox m;

    m.buffer = NULL;
    m.active = false;
    m.charPointer = 0;
    m.charPointer = 0;

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
    box->charTimer = 0;
    box->active = true;

    compute_max_dimensions(msg, &box->width, &box->height);

    return false;
}


void msg_update(MessageBox* box, i16 step){

    if (!box->active) return;

    if (box->charPointer < box->bufferLen) {

        if ((box->charTimer += step) >= CHAR_TIME) {

            box->charTimer -= CHAR_TIME;
            ++ box->charPointer;
        }
    }
}


void msg_draw(MessageBox* box, Bitmap* bmpFont){

    // ...
}


void msg_free_buffer(MessageBox* box) {

    if (box->buffer == NULL)
        return;

    free(box->buffer);
}
