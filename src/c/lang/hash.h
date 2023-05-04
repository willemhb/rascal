#ifndef lang_hash_h
#define lang_hash_h

#include "data/object.h"

// APIs & utilities
#define rl_hash(x)      generic2(hash, x)
#define HASH(x)         Hash[head(x)->type]
#define TYPEHASH(x)     TypeHashes[head(x)->type]

uhash val_hash(value_t vx);
uhash obj_hash(void* ox);
uhash bounded_hash(void* ox, int bound, bool* oob);

// globals
extern uhash TypeHashes[NTYPES];
extern uhash (*Hash[NTYPES])(void* ox, int bound, bool* oob);

#endif
