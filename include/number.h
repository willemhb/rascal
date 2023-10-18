#ifndef rascal_number_h
#define rascal_number_h

#include "object.h"

/* fundamental numeric types and their APIs */

// C types
struct Big {
  Obj     obj;
  /* TODO: change to arbitrary precision. */
  int64_t value;
};

// globals

// external API
bool fits(Value x, int64_t min, int64_t max);


#endif
