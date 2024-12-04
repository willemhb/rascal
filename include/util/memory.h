#ifndef rl_util_memory_h
#define rl_util_memory_h

#include "common.h"

/* tools and utilities for interacting with C heap (Rascal heap defined in runtime.h/c). */

/* external API */
void* s_malloc(size_t n, byte i);
void* s_calloc(size_t n, size_t o, byte i);
void* s_mdup(void* s, size_t n);
char* s_sdup(char* s, size_t n);
void* s_realloc(void* s, size_t p, size_t n, byte i);
void* s_crealloc(void* s, size_t p, size_t n, size_t o, byte i);
void  s_free(void* b);

#endif
