#ifndef rl_util_table_h
#define rl_util_table_h

#include "common.h"

/* Utilities for working with dynamically sizable collections (tables and dynamic arrays). */
/* C types */
typedef enum SizeAlgo : uint8 {
  ALGO_DEFAULT = 0, // use contextual fallback
  ALGO_NONE    = 1, // don't allow resize
  ALGO_STACK   = 2, // power of 2
  ALGO_PYTHON  = 3, // Python list algorithm (still amortized constant but preserves space)
  ALGO_EXACT   = 4, // don't pad
} SizeAlgo;

// collection utilities
size_t adjust_buffer_size(size64 oc, size64 nc, size64 mc, SizeAlgo algo);
size_t adjust_stack_size(size64 oc, size64 nc, size64 mc, SizeAlgo algo);
size_t adjust_table_size(double lf, size_t nc, size_t mc);

#endif
