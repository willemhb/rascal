#ifndef rl_util_table_h
#define rl_util_table_h

#include "common.h"

/* Utilities for working with dynamically sizable collections (tables and dynamic arrays). */
/* C types */
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

/* Initialization */

#endif
