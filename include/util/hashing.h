#ifndef rl_util_hashing_h
#define rl_util_hashing_h

/* Hashing utilities. */
// headers --------------------------------------------------------------------
#include "common.h"

// macros ---------------------------------------------------------------------

// C types --------------------------------------------------------------------

// globals --------------------------------------------------------------------

// function prototypes --------------------------------------------------------
hash_t hash_string(const char* chars);
hash_t hash_word(uintptr_t word);
hash_t hash_pointer(void* ptr);

// initialization -------------------------------------------------------------

#endif
