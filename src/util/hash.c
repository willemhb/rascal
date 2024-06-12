#include "util/hash.h"

/* hashing utilities */
/* Globals */
#define HASH_WIDTH 48
#define HASH_MASK  0x0000ffffffffffffUL

hash_t hash_word(word_t w);
hash_t hash_chars(const char* data);
hash_t hash_bytes(const byte_t* data, size_t count);
hash_t mix_hashes(hash_t hx, hash_t hy);
