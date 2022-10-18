#ifndef rascal_utils_h
#define rascal_utils_h

#include "common.h"

// low-level utilities

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

// string and bytes hashing utilities
hash64_t  hash_string( char *chars );
ord32_t   u16cmp( uint16_t *xb, uint16_t *yb, size_t n );
ord32_t   u32cmp( uint32_t *xb, uint32_t *yb, size_t n );
ord64_t   u64cmp( uint64_t *xb, uint64_t *yb, size_t n );

hash64_t hash_wbytes( uint32_t *wchrs, arity_t cnt );
hash64_t hash_bytes( byte *mem, arity_t cnt );

bool  ihash_bytes( byte **mem, hash64_t **buf, arity_t *cnt, arity_t *cap );
bool  ihash_wbytes( uint32_t **wchrs, hash64_t **buf, arity_t *cnt, arity_t *cap );

// forward declarations & generics

uint64_t clog2(uint64_t i);

// numeric hashing utilities
hash32_t hash_int( uint32_t u );
hash64_t hash_long( uint64_t u );
hash32_t hash_float( float f );
hash64_t hash_double( double r );
hash64_t hash_ptr( void *p );
hash64_t mix_hashes( hash64_t h1, hash64_t h2 );

// convenience
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
