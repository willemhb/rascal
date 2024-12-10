#ifndef rl_util_memory_h
#define rl_util_memory_h

#include "common.h"

/* tools and utilities for interacting with C heap (Rascal heap defined in runtime.h/c). */

/* external API */
void* s_malloc(char* f, size64 n, byte i);
void* s_calloc(char* f, size64 n, size64 o, byte i);
void* s_salloc(char* f, size64 n, size64 o, byte i, bool ae);
void* s_mdup(char* f, void* s, size64 n);
void* s_cdup(char* f, void* s, size64 n, size64 o);
char* s_sdup(char* f, char* s, size64 n);
void* s_realloc(char* f, void* s, size64 p, size64 n, byte i);
void* s_crealloc(char* f, void* s, size64 p, size64 n, size64 o, byte i);
void* s_srealloc(char* f, void* s, size64 p, size64 n, size64 o, byte i, bool ae);
void  s_free(char* f, void* b);

#endif
