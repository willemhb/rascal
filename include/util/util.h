#ifndef rl_util_util_h
#define rl_util_util_h

#include <stdio.h>

#include "common.h"

// miscellaneous file utilities
long filesize(FILE* ios);

// miscellaneous string helpers
bool streq(const char* sx, const char* sy);
bool endswith(const char* str, const char* substr);

// miscellaneous bit twiddling
uintptr_t cpow2(uintptr_t i);

// miscellaneous numeric helpers
bool is_int(Num n);

// hashing functions
hash_t hash_string(const char* chars);
hash_t hash_word(uintptr_t word);
hash_t hash_pointer(const void* ptr);

#endif
