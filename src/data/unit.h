#ifndef unit_h
#define unit_h

#include "base/value.h"

/* the only value of this type is
   nil, but other values are used for
   internal sentinels. */

/* globals */
#define NIL_VAL      (UNIT_TAG|0)

/* API */
bool is_unit(Val x);
bool is_nil(Val x);

#endif
