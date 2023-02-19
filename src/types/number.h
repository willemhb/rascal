#ifndef types_number_h
#define types_number_h

#include "types/value.h"

/* utilities for working with number types */

/* API */
// generic --------------------------------------------------------------------
bool is_number(Val x);
Real as_number(Val x);

#endif
