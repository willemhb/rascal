#ifndef util_hash_h
#define util_hash_h

#include "common.h"

/* C types */
typedef uint64 uhash;

/* API */
uhash hash_uint(uint64 x);
uhash hash_float(double x);
uhash hash_ptr(const void *x);
uhash hash_str(const char *xs);
uhash hash_mem(const ubyte *xs, usize n);
uhash mix_hashes(usize n, ...);

#endif
