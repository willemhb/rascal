#ifndef rl_memory_h
#define rl_memory_h

#include "status.h"

/* tools and utilities for interacting with C heap (Rascal heap defined in runtime.h/c). */

/* external API */

void* rl_malloc(size_t n, byte_t i);
void* rl_calloc(size_t n, size_t o, byte_t i);
void* rl_realloc(size_t p, size_t n, byte_t i);
void* rl_crealloc(size_t p, size_t n, size_t o, byte_t i);
void  rl_free(void* b);

#endif
