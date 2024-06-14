#ifndef rl_number_h
#define rl_number_h

#include "val/object.h"

/* Types, APIs, and globals for representing numbers in Rascal. */
/* C types */
struct Big {
  HEADER;

  int      sign;
  size_t   size;
  byte_t  *digits;
};

struct Ratio {
  HEADER;

  Big* numer;
  Big* denom;
};

/* Globals */
extern Type ArityType, SmallType, RealType, BigType, RatioType;

/* External APIs */
/* Small API */
#define is_small(x) has_type(x, &SmallType)


#endif
