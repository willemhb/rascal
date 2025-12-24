#ifndef rl_util_h
#define rl_util_h

#include "common.h"

// miscellaneous string helpers
bool streq(char* sx, char* sy);
bool endswith(char* str, char* substr);

// miscellaneous bit twiddling
uintptr_t cpow2(uintptr_t i);

// miscellaneous numeric helpers
bool is_int(Num n);

// bitmap utilities
bool bitmap_has(uintptr_t map, int n);
void bitmap_set(uintptr_t* map, int n);
void bitmap_unset(uintptr_t* map, int n);
int bitmap_to_index(uintptr_t map, int n);

// hashing functions
hash_t hash_string(const char* chars);
hash_t hash_word(uintptr_t word);
hash_t hash_pointer(void* ptr);

#endif
