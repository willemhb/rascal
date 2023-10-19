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
extern struct Type FloatType, ArityType, SmallType,
  BigType, NumberType, RealType, RationalType,
  IntegerType;

// external API
Big*  new_big(int64_t value);
bool  fits(Value x, int64_t min, int64_t max);
Type* promote(Value* x, Value* y);

#endif
