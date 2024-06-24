#ifndef rl_lang_compare_h
#define rl_lang_compare_h

#include "val/value.h"

/* Top-level API functions for hashing and comparison */
bool   rl_same(Val x, Val y);
bool   rl_egal(Val x, Val y);
int    rl_order(Val x, Val y);
hash_t rl_hash(Val x, bool f);

#endif
