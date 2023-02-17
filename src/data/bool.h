#ifndef bool_h
#define bool_h

#include "base/value.h"

/* globals */
#define TRUE_VAL  (BOOL_TAG|1)
#define FALSE_VAL (BOOL_TAG|0)

/* API */
bool is_bool(Val x);
Bool as_bool(Val x);
Val  mk_bool(Bool x);

bool C_bool(Val x);

#endif
