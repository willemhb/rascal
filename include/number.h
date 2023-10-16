#ifndef rascal_number_h
#define rascal_number_h

#include "object.h"

/* fundamental numeric types and their APIs */

struct Big {
  Obj     obj;
  /* TODO: change to arbitrary precision. */
  int64_t value;
};


#endif
