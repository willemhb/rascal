#ifndef rascal_util_number_h
#define rascal_util_number_h

#include "common.h"

// generic min/max macros
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

// general numeric utilities
uint64_t ceilPow2(uint64_t word);

#endif
