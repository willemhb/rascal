#ifndef rl_util_memory_h
#define rl_util_memory_h

#include "error.h"

/* tools and utilities for interacting with C heap (Rascal heap defined in runtime.h/c). */

/* external API */
void* rl_malloc(size_t n, byte i);
void* rl_calloc(size_t n, size_t o, byte i);
void* rl_mdup(void* s, size_t n);
char* rl_sdup(char* s, size_t n);
void* rl_realloc(void* s, size_t p, size_t n, byte i);
void* rl_crealloc(void* s, size_t p, size_t n, size_t o, byte i);
void  rl_free(void* b);

#endif
