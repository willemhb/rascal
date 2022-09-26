
#ifndef rascal_utils_num_h
#define rascal_utils_num_h

#include "core.h"

// C types --------------------------------------------------------------------
typedef enum
  {
    Ctype_sint8=1, Ctype_uint8,
    Ctype_sint16,  Ctype_uint16,
    Ctype_sint32,  Ctype_uint32,   Ctype_float32,
    Ctype_sint64,  Ctype_uint64,   Ctype_pointer, Ctype_float64
  } Ctype_t;

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
  real_t   ieee;

  struct
  {
    uint64_t frac : 52;
    uint64_t expt : 11;
    uint64_t sign :  1;
  } parts;
} ieee64_t;

// forward declarations & generics --------------------------------------------
size_t   sizeof_Ctype( Ctype_t Ctype );
bool     Ctype_fits( Ctype_t Cx, Ctype_t Cy );
Ctype_t  common_Ctype( Ctype_t Cx, Ctype_t Cy );

uint64_t clog2(uint64_t i);

// numeric hashing utilities --------------------------------------------------
hash_t hash_int( uint64_t u );
hash_t hash_real( real_t r );
hash_t hash_ptr( ptr_t p );
hash_t mix_hashes( hash_t h1, hash_t h2 );

// ordering helpers
int    ord_ulong( uint64_t x, uint64_t y );

// convenience ----------------------------------------------------------------
#define real_bits(r)  (((ieee64_t)(r)).bits)
#define float_bits(f) (((ieee32_t)(f)).bits)

#define Ctypeof(type)					\
  _Generic((type)0,					\
	   int8_t:   Ctype_sint8,			\
	   uint8_t:  Ctype_uint8,			\
	   int16_t:  Ctype_sint16,			\
	   uint16_t: Ctype_uint16,			\
	   int32_t:  Ctype_sint32,			\
	   uint32_t: Ctype_uint32,			\
	   float:    Ctype_float32,			\
	   int64_t:  Ctype_sint64,			\
	   uint64_t: Ctype_uint64,			\
	   double:   Ctype_float64,			\
	   void*:    Ctype_pointer)

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

#endif
