#ifndef __ERROR__
#define __ERROR__

#include "types.h"

#define ERROR_MAX_LENGTH 64

#define ERR_MALLOC() err_throw("Memory allocation error!");

void err_init();

void err_throw(const str msg);
void err_throw_str(const str msg, str param);

bool err_has_any();
bool err_pop(str buffer);

#endif // __ERROR__
