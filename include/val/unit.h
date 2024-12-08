#ifndef rl_val_unit_h
#define rl_val_unit_h

#include "val/value.h"

/* C types */

/* Globals */
/* not a real value (shouldn't leak), but used to detect absent or uninitialized
   values where a value is expected. Eg, unused table nodes have both their key and
   value set to NOTHING, while tombstones have their key set to NOTHING but retain
   a valid value. */
#define NOTHING    0xffff001000000001ul // UNIT  |  1
#define NUL        0xffff001000000000ul // UNIT  |  0

/* API */
#define is_nul(x) ((x) == NUL)
#define as_nul(x) NUL

/* Initialization */

#endif
