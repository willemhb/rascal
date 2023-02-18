#ifndef lang_compare_h
#define lang_compare_h

#include "base/value.h"
#include "base/type.h"

/* globals */
extern bool (*Equal[NUM_TYPES])(Val x, Val y, void* state);
extern int  (*Compare[NUM_TYPES])(Val x, Val y, void* state);

/* API */
bool same(Val x, Val y);
bool equal(Val x, Val y);
int  compare(Val x, Val y);

#endif
