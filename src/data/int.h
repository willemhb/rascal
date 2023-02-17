#ifndef data_int_h
#define data_int_h

#include "base/value.h"
#include "base/object.h"

#include "util/hash.h"

/* API */
bool is_int(Val x);
Int as_int(Val x);
Val tag_int(Int i);

void print_int(Val x);
uhash hash_int(Val x, int bound, uhash* acc);
bool equal_ints(Val x, Val y, int bound, Table* backedges);
int compare_ints(Val x, Val y, int bound, Table* backedges);

#endif
