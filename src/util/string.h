#ifndef rascal_util_string_h
#define rascal_util_string_h

#include <string.h>

#include "../common.h"



/* API */
#define streq(x, y) generic((x), char*: streq8)(x, y)

bool streq8(const char *s1, const char *s2);

#endif
