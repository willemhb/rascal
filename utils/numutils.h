#ifndef numutils_h
#define numutils_h

#include <stdbool.h>
#include <stdint.h>

#define BIT64 0x8000000000000000ul
#define BIT32 0x80000000u

#define min( _x, _y)				\
  ({						\
    typeof(_x) _x_ = _x;			\
    typeof(_y) _y_ = _y;			\
    _x_ > _y_ ? _y_ : _x_;			\
  })

#define max( _x, _y)				\
  ({						\
    typeof(_x) _x_ = _x;			\
    typeof(_y) _y_ = _y;			\
    _x_ < _y_ ? _y_ : _x_;			\
  })

#define rot(x,k)				\
  ({						\
    typeof(x) __x = x;				\
    typeof(k) __k = k;				\
    (__x<<__k) | (__x >> 32-__k);		\
  })

typedef union {
  bool as_bool;
  int32_t as_int32;
  uint32_t as_uint32;
  float as_float;
} bits32_t;

#define get_int32( _v ) (((bits32_t)(_v)).as_int32)
#define get_uint32( _v ) (((bits32_t)(_v)).as_uint32)
#define get_float( _v ) (((bits32_t)(_v)).as_float)
#define get_bool( _v )  (((bits32_t)(_v)).as_bool)


typedef union {
  uint32_t bin;
  float fp;

  struct {
    uint32_t sign :  1;
    uint32_t expt :  8;
    uint32_t frac : 23;
  };
} ieee32_t;

typedef union {
  uint64_t bin;
  double fp;

  struct {
    uint64_t sign :  1;
    uint64_t expt : 11;
    uint64_t frac : 52;
  };
} ieee64_t;

// exports -------------------------------------------------------------------
uint64_t nextipow2( uint64_t u );
uint16_t bswap16( uint16_t u );
uint32_t bswap32( uint32_t u );
uint64_t bswap64( uint64_t u );

int  ord_int( int xi, int yi );
int  ord_uint( uint32_t ux, uint32_t uy );
int  ord_long( long xi, long yi );
int  ord_ulong( uintptr_t ux, uintptr_t uy );

/*
int      fp32sign( float xf );
uint32_t fp32expt( float xf );
uint32_t fp32frac( float xf );

int fp64sign( double xd );
uint32_t fp64expt( double xd );
uint32_t fp64frac( double xd );

#define fpsign( x )				\
  _Generic((x),					\
	   float:fp32sign,			\
	   double:fp64sign)(x)

#define fpexpt( x )				\
  _Generic((x),					\
	   float:fp32expt,			\
	   double:fp64expt)(x)

#define fpfrac( x )				\
  _Generic((x),					\
	   float:fp32frac,			\
	   double:fp64frac)(x)
*/

#endif
