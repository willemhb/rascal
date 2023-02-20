#ifndef compare_h
#define compare_h

#include "value.h"

// C types --------------------------------------------------------------------
typedef uint64 uhash;

// API ------------------------------------------------------------------------
bool  same(Val x, Val y);
bool  equal(Val x, Val y);
int   compare(Val x, Val y);
uhash hash(Val x, bool deep);

#endif
