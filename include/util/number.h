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

#define popc(n)                                 \
  _Generic((n),                                 \
           uint32_t:__builtin_popcount,         \
           uint64_t:__builtin_popcountl)(n)

#define ctz(n)                                  \
  _Generic((n),                                 \
           uint32_t:__builtin_ctz,              \
           uint64_t:__builtin_ctzl)(n)

#define clz(n)                                  \
  _Generic((n),                                 \
           uint32_t:__builtin_clz,              \
           uint64_t:__builtin_clzl)(n)

// general numeric utilities
uint64_t ceil2(uint64_t word);

#endif
