#ifndef rl_memory_h
#define rl_memory_h

#include "status.h"

/* tools and utilities for interacting with C heap (Rascal heap defined in runtime.h/c). */

/* external API */

rl_status_t rl_malloc(void** b, size_t n, byte_t i);
rl_status_t rl_calloc(void** b, size_t n, size_t o, byte_t i);
rl_status_t rl_realloc(void** b, size_t p, size_t n, byte_t i);
rl_status_t rl_crealloc(void** b, size_t p, size_t n, size_t o, byte_t i);
rl_status_t rl_free(void* b);

#endif
