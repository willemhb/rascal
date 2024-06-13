#ifndef rl_util_number_h
#define rl_util_number_h

#include "common.h"

/* Rascal numeric utilities. */
// generic min, max, and compare macros

#define cmp(x, y)                               \
  ({                                            \
    __auto_type _x = x;                         \
    __auto_type _y = y;                         \
    0 - (_x < _y) + (_x > _y);                  \
  })

#define min(x, y)                               \
  ({                                            \
    __auto_type _x = x;                         \
    __auto_type _y = y;                         \
    _x > _y ? _y : _x;                          \
  })

#define max(x, y)                               \
  ({                                            \
    __auto_type _x = x;                         \
    __auto_type _y = y;                         \
    _x < _y ? _y : _x;                          \
  })

/* Next largest power of 2. */
word_t ceil2(word_t w);

#endif
