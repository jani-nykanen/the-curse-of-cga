#include "keyb.h"

#include <dos.h>
#include <conio.h>
#include <bios.h>

#include <stdlib.h>
#include <stdio.h>

#define KEY_BUFFER_SIZE 0x60


static u8 normalKeys [KEY_BUFFER_SIZE];
static u8 extKeys [KEY_BUFFER_SIZE];

static u8 oldNormals [KEY_BUFFER_SIZE];
static u8 oldExt [KEY_BUFFER_SIZE];

static bool normalRead [KEY_BUFFER_SIZE];
static bool extRead [KEY_BUFFER_SIZE];


static void far interrupt (*oldHandler) (void) = NULL;


static void far interrupt handler() {

    static u8 buffer = 0;
    u8 rawcode;
    u8 makeBreak;
    i16 scancode;

    rawcode = inp(0x60); 
    makeBreak = !(rawcode & 0x80); 
    scancode = rawcode & 0x7F;

    if (buffer == 0xE0) { 

        if(scancode < 0x60) {

            oldExt[scancode] = extKeys[scancode];
            extKeys[scancode] = makeBreak;

            if (extKeys[scancode] != oldExt[scancode])
                extRead[scancode] = false;
        }
        buffer = 0;
    } 
    else if (buffer >= 0xE1 && buffer <= 0xE2) {

        buffer = 0;
    } 
    else if (rawcode >= 0xE0 && rawcode <= 0xE2) {

        buffer = rawcode; 
    } 
    else if (scancode < 0x60) {

        oldNormals[scancode] = normalKeys[scancode];
        normalKeys[scancode] = makeBreak;

        if (normalKeys[scancode] != oldNormals[scancode])
            normalRead[scancode] = false;
    }

    outp(0x20, 0x20);
}


static i8 get_value_from_array(u8* arr, bool* readArr, i16 id) {

    u8 state = arr[id];
    bool read = readArr[id];
    u8 ret = 0;

    if(state == STATE_DOWN) {

        ret = read ? STATE_DOWN : STATE_PRESSED;
    }
    else if(state == STATE_UP) {

        ret = read ? STATE_UP : STATE_RELEASED;
    }
    readArr[id] = true;

    return ret;
}




void init_keyboard_listener() {

    oldHandler = _dos_getvect(0x09);
    _dos_setvect(0x09, handler);
}


void reset_keyboard_listener() {

    if (oldHandler != NULL) {
        
        _dos_setvect(0x09, oldHandler);
        oldHandler = NULL;
    }
}


State keyb_get_normal_key(u8 key) {

    if(key > KEY_BUFFER_SIZE)
        return STATE_UP;

    return get_value_from_array(
        normalKeys, 
        normalRead, key);
}


State keyb_get_ext_key(u8 key) {

    if(key > KEY_BUFFER_SIZE)
        return STATE_UP;

    return get_value_from_array(
        extKeys, 
        extRead, key);
}
