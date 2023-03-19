#ifndef number_h
#define number_h

#include "value.h"

/* numeric utilities & builtin numeric types */

// C types --------------------------------------------------------------------
struct complex_t {
  HEADER;
  real_t real;
  real_t imag;
};

struct ratio_t {
  HEADER;
  value_t numer; // might be a fixnum or a big int
  value_t denom;
};

struct big_t {
  HEADER;
  sint32 sign;
  uint32 length;     // digit count
  ubyte  digits[];   // digit values
};

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

#define CMP(x, y)                                \
  ({                                             \
    __auto_type _x = x;                          \
    __auto_type _y = y;                          \
    int _r = 0;                                  \
    if (_x < _y) _r = -1;                        \
    if (_x > _y) _r =  1;                        \
    _r;                                          \
  })

// API ------------------------------------------------------------------------
uword  dtow(double dbl);
double wtod(uword word);
uword  ceil2(uword word);

#endif
