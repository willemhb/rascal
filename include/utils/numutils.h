#ifndef rascal_utils_numutils_h
#define rascal_utils_numutils_h

#include "common.h"

// floating point manipulation
typedef union
{
  uint32_t bits;
  float    ieee;

  struct
  {
    uint32_t frac : 23;
    uint32_t expt :  8;
    uint32_t sign :  1;
  };
} ieee32_t;


typedef union
{
  uint64_t bits;
  double   ieee;

  struct
  {
    uint64_t frac : 52;
    uint64_t expt : 11;
    uint64_t sign :  1;
  } parts;
} ieee64_t;

#define real_bits(r)  (((ieee64_t)(r)).bits)
#define float_bits(f) (((ieee32_t)(f)).bits)

// generic comparison
#define cmp(a, b)				\
  ({						\
    typeof(a) _a_ = a;				\
    typeof(b) _b_ = b;				\
    0 - (_a_<_b_) + (_a_>_b_);			\
  })

#define max(a, b)				\
  ({						\
    typeof(a) _a_ = a;				\
    typeof(b) _b_ = b;				\
    _a_ < _b_ ? _b_ : _a_;			\
  })

#define min(a, b)				\
  ({						\
    typeof(a) _a_ = a;				\
    typeof(b) _b_ = b;				\
    _a_ > _b_ ? _b_ : _a_;			\
  })

// generic bit manipulation
#define clz(i)					\
  _Generic((i),					\
	   int32_t:__builtin_clz,		\
	   uint32_t:__builtin_clz,		\
	   int64_t:__builtin_clzl,		\
	   uint64_t:__builtin_clzl)(i)

#define ctz(i)					\
  _Generic((i),					\
	   int32_t:__builtin_ctz,		\
	   uint32_t:__builtin_ctz,		\
	   int64_t:__builtin_ctzl,		\
	   uint64_t:__builtin_ctzl)(i)

#define popcnt(i)				\
  _Generic((i),					\
	   int32_t:__builtin_popcount,		\
	   uint32_t:__builtin_popcount,		\
	   int64_t:__builtin_popcountl,		\
	   uint64_t:__builtin_popcountl)(i)

#define bswap(i)				\
  _Generic((i),					\
	   int16_t:__builtin_bswap16,		\
	   uint16_t:__builtin_bswap16,		\
	   int32_t:__builtin_bswap32,		\
	   uint32_t:__builtin_bswap32,		\
	   int64_t:__builtin_bswap64,		\
	   uint64_t:__builtin_bswap64)(i)

// miscellaneous bit manipulation
uint64_t clog2(uint64_t i);

#endif
