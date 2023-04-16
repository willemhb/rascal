#ifndef data_unit_h
#define data_unit_h

#include "data/value.h"

// APIs & utilities
#define is_unit(x) rl_isa(x, UNIT)

// globals
#define UNITTAG    (IMMTAG | (((uword)UNIT) << 32))

#define NUL        (UNITTAG | 0)
#define NOTFOUND   (UNITTAG | 1)
#define UNDEFINED  (UNITTAG | 3)
#define UNBOUND    (UNITTAG | 5)

#endif
