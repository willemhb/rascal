#ifndef lang_hash_h
#define lang_hash_h

#include "types/value.h"

/* C types */
typedef uint64 uhash;

/* API */
uhash hash(Val x, bool deep);

#endif
