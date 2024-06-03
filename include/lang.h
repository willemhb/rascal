#ifndef rl_lang_h
#define rl_lang_h

#include "value.h"

/* C functions and types for implementing core language operations. */
bool   rl_same(Value x, Value y);
bool   rl_egal(Value x, Value y);
int    rl_order(Value x, Value y);
hash_t rl_hash(Value x, bool fast);

#endif
