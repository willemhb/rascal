#ifndef compare_h
#define compare_h

#include "value.h"

// API ------------------------------------------------------------------------
bool  same(value_t x, value_t y);
bool  equal(value_t x, value_t y);
int   compare(value_t x, value_t y);
uhash hash(value_t x);

// initialization -------------------------------------------------------------
void compare_init(void);

#endif
