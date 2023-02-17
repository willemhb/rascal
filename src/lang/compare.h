#ifndef compare_h
#define compare_h

#include "value.h"

/* API */
// value comparisons ----------------------------------------------------------
bool same(Val x, Val y);
bool equal(Val x, Val y);
int  compare(Val x, Val y);

#endif
