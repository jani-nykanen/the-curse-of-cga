#ifndef __MESSAGE_BOX__
#define __MESSAGE_BOX__


#include "types.h"
#include "bitmap.h"

typedef struct {

    str buffer;
    u16 bufferLen;
    u16 charPointer;
    u16 lastCharPointer;
    i16 charTimer;
    u16 width;
    u16 height;
    bool active;
    bool boxDrawn;

} MessageBox;


MessageBox create_message_box();

bool msg_build(MessageBox* box, const str msg);

bool msg_update(MessageBox* box, i16 step);
void msg_draw(MessageBox* box, Bitmap* bmpFont,
    i16 x, i16 y, i16 w, i16 h);

void msg_free_buffer(MessageBox* box);

#endif // __MESSAGE_BOX__
