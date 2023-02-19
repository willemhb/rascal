#ifndef lang_compare_h
#define lang_compare_h

#include "types/value.h"

/* API */
bool same(Val x, Val y);
bool equal(Val x, Val y);
int  compare(Val x, Val y);

#endif
