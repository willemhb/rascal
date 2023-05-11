#include "data/value.h"
#include "data/object.h"


type_t value_type(value_t x) {
  switch (x & WTMASK) {
    case IMMTAG: return x >> 32 & 63;
    case FIXTAG: return FIXNUM;
    case OBJTAG: return OBJECT;
    default:     return REAL;
  }
}


type_t rascal_type(value_t x) {
  type_t vtype = value_type(x);

  if (vtype == OBJECT)
    vtype = object_type(as_object(x));

  return vtype;
}


