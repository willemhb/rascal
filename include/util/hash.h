#ifndef rl_util_hash_h
#define rl_util_hash_h

#include "common.h"

/* Basic hashing library. */
// hashing utilities
hash_t hash_word(word_t w);
hash_t hash_chars(const char* data);
hash_t hash_bytes(const byte_t* data, size_t count);
hash_t mix_hashes(hash_t hx, hash_t hy);

#endif
