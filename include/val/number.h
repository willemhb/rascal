#ifndef rl_number_h
#define rl_number_h

#include "val/object.h"

/* Types, APIs, and globals for representing numbers in Rascal. */
/* C types */

/* Globals */
extern Type ArityType, SmallType, RealType;

/* External APIs */
/* Small API */
#define is_small(x) has_type(x, &SmallType)

#endif
