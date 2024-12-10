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
// array resizing
bool   check_alist_grow(size64 m, size64 n);
bool   check_alist_shrink(size64 m, size64 n);
bool   check_buffer_grow(size64 m, size64 n);
bool   check_buffer_shrink(size64 m, size64 n);
bool   check_table_grow(size64 m, size64 n);
bool   check_table_shrink(size64 m, size64 n);
size64 adjust_alist_size(size64 n);
size64 adjust_buffer_size(size64 n, bool ae);
size64 adjust_table_size(size64 n);

// alloc/realloc
size64 copy_array(void* dst, void* src, size64 n, size64 os);
void*  alloc_array(char* fn, void* ini, size64 n, size64 os);
void*  realloc_array(char* fn, void* arr, size64 on, size64 nn, size64 os);
void*  trim_array(char* fn, void* arr, size64 n, size64 os, bool cpy);
void*  pad_array(char* fn, void* arr, size64 n, size64 e, size64 os, bool cpy);
void*  alloc_string(char* fn, void* ini, size64 n, size64 os, bool ae);
void*  realloc_string(char* fn, void* str, size64 on, size64 nn, size64 os, bool ae);
void*  trim_string(char* fn, void* str, size64 n, size64 os, bool cpy, bool ae);
void*  pad_string(char* fn, void* str, size64 n, size64 e, size64 os, bool cpy, bool ae);
void*  alloc_alist(char* fn, void* ini, size64 n, size64 os, size64 *m);
void*  realloc_alist(char* fn, void* arr, size64 on, size64 nn, size64 os, size64* m);
void*  alloc_buffer(char* fn, void* ini, size64 n, size64 os, size64* m, bool ae);
void*  realloc_buffer(char* fn, void* buf, size64 on, size64 nn, size64 os, size64* m, bool ae);
void*  alloc_table(char* fn, size64 n, size64 os, TInitFn init, size64* m);
void*  realloc_table(char* fn, void* kvs, size64 om, size64 nn, size64 os, TRehashFn rehash, size64 *m);

/* Initialization */

#endif
