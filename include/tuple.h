#ifndef rascal_tuple_h
#define rascal_tuple_h

#include "core.h"

struct tuple_t {
  object_t base;
  size_t len, cap;
  
  value_t *vals;
};

// utilities ------------------------------------------------------------------
#define astup(x)   ((tuple_t*)ptr(x))
#define tlen(x)    (astup(x)->len)
#define tcap(x)    (astup(x)->cap)
#define tvals(x)   (astup(x)->vals)

// api ------------------------------------------------------------------------

#endif
