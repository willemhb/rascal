#ifndef rl_lang_compare_h
#define rl_lang_compare_h

#include "val/value.h"

/* Top-level API functions for hashing and comparison */
bool   rl_same(Value x, Value y);
bool   rl_egal(Value x, Value y);
int    rl_order(Value x, Value y);
hash_t rl_hash(Value x, bool fast);

#endif
