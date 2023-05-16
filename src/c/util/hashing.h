#ifndef util_hashing_h
#define util_hashing_h

#include "util/number.h"

// API ------------------------------------------------------------------------
uhash hash_uword(uword word);
uhash hash_double(double dbl);
uhash hash_pointer(void* ptr);
uhash hash_str(char* chars);
uhash hash_mem(ubyte* bytes, usize n);
uhash mix_2_hashes(uhash x, uhash y);
uhash mix_3_hashes(uhash x, uhash y, uhash z);
uhash mix_n_hashes(usize n, ...);

#endif
