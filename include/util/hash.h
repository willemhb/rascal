#ifndef rl_util_hash_h
#define rl_util_hash_h

#include "common.h"

/* Basic hashing library. */
// hashing utilities
hash64 hash_word(word_t w);
hash64 hash_chars(const char* d, size64 n);
hash64 hash_bytes(const byte* d, size64 n);
hash64 hash_pointer(const void* p);
hash64 mix_hashes(hash64 x, hash64 y);

#endif
