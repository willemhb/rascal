#ifndef rl_util_hashing_h
#define rl_util_hashing_h

#include "common.h"

// hashing functions
hash_t hash_string(const char* chars);
hash_t hash_bytes(const uint8_t* bytes, size_t n);
hash_t hash_word(uint64_t word);
hash_t hash_ptr(const void* pointer);
hash_t hash_double(double num);
hash_t mix_hashes(hash_t hx, hash_t hy);

#endif
