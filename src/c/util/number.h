#ifndef number_h
#define number_h

#include "common.h"

// C types --------------------------------------------------------------------
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
