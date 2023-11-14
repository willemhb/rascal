#ifndef rl_equal_h
#define rl_equal_h

#include "common.h"

// external API
bool   same(Value x, Value y);
bool   equal(Value x, Value y);
int    order(Value x, Value y);
hash_t hash(Value x);

#endif
