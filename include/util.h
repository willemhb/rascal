#ifndef rl_util_h
#define rl_util_h

#include "common.h"

// miscellaneous bit twiddling
uintptr_t cpow2(uintptr_t i);

// hashing functions
hash_t hash_string(char* chars);
hash_t hash_word(uintptr_t word);
hash_t hash_pointer(void* ptr);

#endif
