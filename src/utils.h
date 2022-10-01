#ifndef rascal_utils_h
#define rascal_utils_h

#include "common.h"

// low-level utilities

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
  double   ieee;

  struct
  {
    uint64_t frac : 52;
    uint64_t expt : 11;
    uint64_t sign :  1;
  } parts;
} ieee64_t;

// C types --------------------------------------------------------------------
typedef enum
  {
    enc_ascii =16|Ctype_sint8,
    enc_latin1=16|Ctype_uint8,
    enc_utf8  =32|Ctype_sint8,
    enc_utf16 =16|Ctype_sint16,
    enc_utf32 =16|Ctype_sint32
  } encoding_t;

#define enc_mask 0x3f

// string and bytes hashing utilities
hash_t hash_string( char *chars );
int    u64cmp( uint64_t *xb, uint64_t *yb, size_t n );
int    u32cmp( uint32_t *xb, uint32_t *yb, size_t n );
int    u16cmp( uint16_t *xb, uint16_t *yb, size_t n );

hash_t hash_wbytes( uint32_t *wchrs, arity_t cnt );
hash_t hash_bytes( byte_t *mem, arity_t cnt );

bool  ihash_bytes( byte_t **mem, hash_t **buf, arity_t *cnt, arity_t *cap );
bool  ihash_wbytes( uint32_t **wchrs, hash_t **buf, arity_t *cnt, arity_t *cap );

// forward declarations & generics
size_t   sizeof_Ctype( Ctype_t Ctype );
bool     Ctype_fits( Ctype_t Cx, Ctype_t Cy );
Ctype_t  common_Ctype( Ctype_t Cx, Ctype_t Cy );

uint64_t clog2(uint64_t i);

// numeric hashing utilities
hash_t hash_int( uint64_t u );
hash_t hash_double( double r );
hash_t hash_ptr( void *p );
hash_t mix_hashes( hash_t h1, hash_t h2 );

// convenience
size_t sizeof_Ctype( Ctype_t C )
{ 
  switch(C)
    {
    case Ctype_sint8  ... Ctype_uint8:    return 1;
    case Ctype_sint16 ... Ctype_uint16:   return 2;
    case Ctype_sint32 ... Ctype_float32:  return 4;
    default:                              return 8;
    }
}

static inline size_t sizeof_enc(encoding_t enc)
{
  return sizeof_Ctype(enc&15);
}

static inline bool is_multibyte( encoding_t enc )
{
  return enc == enc_utf8 || enc == enc_utf16;
}

static inline Ctype_t enc_Ctype( encoding_t enc )
{
  return enc&15;
}

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

// array & map utilities
size_t pad_alist_size(size_t oldl, size_t newl, size_t oldc, size_t minc);
size_t pad_stack_size(size_t oldl, size_t newl, size_t oldc, size_t minc);
size_t pad_table_size(size_t oldl, size_t newl, size_t oldc, size_t minc);

// genericized array operations (NB: not portable, change these to macros at some point)


#endif
