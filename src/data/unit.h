#ifndef unit_h
#define unit_h

#include "value.h"

/* the only value of this type is
   nil, but other values are used for
   internal sentinels. */

/* globals */
#define LT_VAL       (UNIT_TAG|-1)
#define EQ_VAL       (UNIT_TAG|0)
#define GT_VAL       (UNIT_TAG|1)
#define NIL_VAL      (UNIT_TAG|2)
#define UNDEF_VAL    (UNIT_TAG|3)
#define UNBOUND_VAL  (UNIT_TAG|4)
#define NOTFOUND_VAL (UNIT_TAG|5)

/* API */
bool is_unit(Val x);
bool is_nil(Val x);

#endif
