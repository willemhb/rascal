#ifndef number_h
#define number_h

#include <stdint.h>
#include <stddef.h>

// C types --------------------------------------------------------------------
typedef uint8_t     ubyte;
typedef uint32_t    uint;
typedef uint16_t    ushort;
typedef uintptr_t   uword;
typedef uintptr_t   uhash;
typedef size_t      usize;
typedef void      (*funcptr)(void);
typedef uint32_t    flags;

typedef uint8_t  uint8;
typedef int8_t   sint8;
typedef uint16_t uint16;
typedef int16_t  sint16;
typedef uint32_t uint32;
typedef int32_t  sint32;
typedef uint64_t uint64;
typedef int64_t  sint64;

typedef union {
  double dbl;
  uword  word;
  struct {
    uword frac : 52;
    uword expt : 11;
    uword sign :  1;
  } parts;
} ieee64_t;


// generic max/min/cmp --------------------------------------------------------

#define MAX(x, y)                               \
  ({                                            \
    __auto_type __x = x;                        \
    __auto_type __y = y;                        \
    __x < __y ? __y : __x;                      \
  })

#define MIN(x, y)                               \
  ({                                            \
    __auto_type __x = x;                        \
    __auto_type __y = y;                        \
    __x > __y ? __y : __x;                      \
  })

#define CMP(x, y)                               \
  ({                                            \
    __auto_type __x = x;                        \
    __auto_type __y = y;                        \
    0 - (__x < __y) + (__x > __y);              \
  })

// other utilities
uint64 ceil2(uint64 i);

#endif
