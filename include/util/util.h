#ifndef rl_util_util_h
#define rl_util_util_h

#include <stdio.h>

#include "common.h"

// miscellaneous file utilities
long filesize(FILE* ios);

// miscellaneous string helpers
bool streq(const char* sx, const char* sy);
bool endswith(const char* str, const char* substr);
bool startswith(const char* str, const char* substr);

// miscellaneous bit twiddling
uintptr_t cpow2(uintptr_t i);

// miscellaneous numeric helpers
#define max(x, y)                               \
  ({                                            \
    typeof(x) __x__ = (x);                      \
    typeof(y) __y__ = (y);                      \
    __x__ < __y__ ? __y__ : __x__;              \
  })

#define min(x, y)                               \
  ({                                            \
    typeof(x) __x__ = (x);                      \
    typeof(y) __y__ = (y);                      \
    __x__ > __y__ ? __y__ : __x__;              \
  })

bool is_int(Num n);

// hashing functions
hash_t hash_48(hash_t h); // compress to 48 bits
hash_t hash_string(const char* chars);
hash_t hash_string_48(const char* chars);
hash_t hash_word(uintptr_t word);
hash_t hash_word_48(uintptr_t word);
hash_t hash_pointer(const void* ptr);
hash_t hash_pointer_48(const void* ptr);
hash_t mix_hashes(hash_t hx, hash_t hy);
hash_t mix_hashes_48(hash_t hx, hash_t hy);

#endif
