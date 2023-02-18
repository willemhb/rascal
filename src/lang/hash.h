#ifndef lang_hash_h
#define lang_hash_h

#include "base/value.h"
#include "base/type.h"

#include "util/hash.h"

/* globals */
extern uhash (*Hash[NUM_TYPES])(Val x, void* state);

/* API */
uhash hash(Val x, bool deep_hash);

#endif
