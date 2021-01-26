#include "err.h"

#include <stdio.h>

#define MAX_ERR_COUNT 8

static char errorMessages [MAX_ERR_COUNT] [ERROR_MAX_LENGTH];
static u8 errCount = 0;


void err_init() {

    errCount = 0;
}


void err_throw(const str msg) {

    if (errCount == MAX_ERR_COUNT)
        return;

    snprintf(errorMessages[errCount ++], ERROR_MAX_LENGTH, "%s", msg);
}


void err_throw_str(const str msg, str param) {

    if (errCount == MAX_ERR_COUNT)
        return;

    snprintf(errorMessages[errCount ++], ERROR_MAX_LENGTH, "%s%s", msg);
}


bool err_has_any() {

    return errCount > 0;
}


bool err_pop(str buffer) {

    if (errCount == 0) 
        return false;

    snprintf(buffer, ERROR_MAX_LENGTH, "%s", errorMessages[-- errCount]);
    return true;
}
