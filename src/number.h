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

#define MIN(x, y)                               \
  ({                                            \
    __auto_type _x = x;                         \
    __auto_type _y = y;                         \
    _x > _y ? _y : _x;                          \
  })


#define MAX(x, y)                               \
  ({                                            \
    __auto_type _x = x;                         \
    __auto_type _y = y;                         \
    _x < _y ? _y : _x;                          \
  })

// API ------------------------------------------------------------------------
uword  dtow(double dbl);
double wtod(uword word);
uword  ceil2(uword word);
uword  hash_uword(uword word);

#endif
