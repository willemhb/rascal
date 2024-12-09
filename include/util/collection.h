#ifndef rl_util_collection_h
#define rl_util_collection_h

#include "common.h"

/* Utilities for working with dynamically sizable collections (tables and dynamic arrays). */
/* C types */
typedef void (*TInitFn)(void* spc, size64 m);
typedef void (*TRehashFn)(void* ospc, size64 om, void* nspc, size64 nm);
// typedef enum SizeAlgo : uint8 {
//  ALGO_DEFAULT = 0, // use contextual fallback
//  ALGO_NONE    = 1, // don't allow resize
//  ALGO_STACK   = 2, // power of 2
//  ALGO_PYTHON  = 3, // Python list algorithm (still amortized constant but preserves space)
//  ALGO_EXACT   = 4, // don't pad
// } SizeAlgo;

/* Globals */

/* API */
bool   check_alist_grow(size64 m, size64 n);
bool   check_alist_shrink(size64 m, size64 n);
bool   check_buffer_grow(size64 m, size64 n);
bool   check_buffer_shrink(size64 m, size64 n);
bool   check_table_grow(size64 m, size64 n);
bool   check_table_shrink(size64 m, size64 n);
size64 adjust_alist_size(size64 n);
size64 adjust_buffer_size(size64 n);
size64 adjust_table_size(size64 n);
void*  alloc_array(void* ini, size64 n, size64 os);
void*  realloc_array(void* arr, size64 on, size64 nn, size64 os);
void*  alloc_string(void* ini, size64 n, size64 os);
void*  realloc_string(void* str, size64 on, size64 nn, size64 os);
void*  alloc_alist(void* ini, size64 n, size64 os, size64 *m);
void*  realloc_alist(void* arr, size64 on, size64 nn, size64 os, size64* m);
void*  alloc_buffer(void* ini, size64 n, size64 os, size64* m);
void*  realloc_buffer(void* buf, size64 on, size64 nn, size64 os, size64* m);
void*  alloc_table(size64 n, size64 os, TInitFn init, size64* m);
void*  realloc_table(void* kvs, size64 on, size64 nn, size64 os, TRehashFn rehash, size64 *m);

/* Initialization */

#endif
