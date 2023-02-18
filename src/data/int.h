#ifndef data_int_h
#define data_int_h

#include "base/value.h"

/* API */
bool is_int(Val x);
Int  as_int(Val x);
Val  mk_int(Int i);

#endif
