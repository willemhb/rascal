#ifndef rl_util_bits_h
#define rl_util_bits_h

#include "common.h"

/* Bit twiddling helpers. */
#define popc(x)                                 \
  generic((x),                                  \
          int32_t:__builtin_popcount,           \
          uint32_t:__builtin_popcount,          \
          int64_t:__builtin_popcountl,          \
          uint64_t:__builtin_popcountl)(x)

#define ctz(x)                                  \
  generic((x),                                  \
          sint32:__builtin_ctz,                \
          uint32:__builtin_ctz,                \
          sint64:__builtin_ctzl,               \
          uint64:__builtin_ctzl)(x)

#endif
