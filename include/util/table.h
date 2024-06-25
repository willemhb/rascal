#ifndef rl_util_table_h
#define rl_util_table_h

#include "common.h"

/* Utilities for working with dynamically sizable collections (tables and dynamic arrays). */
/* C types */
typedef enum {
  RESIZE_STACK  = 0, // power of 2
  RESIZE_PYTHON = 1, // Python list algorithm (still amortized constant but preserves space)
  RESIZE_EXACT  = 2, // don't pad
} ResizeAlgo;

// collection utilities
size_t adjust_buffer_size(size_t oc, size_t nc, size_t mc, ResizeAlgo algo);
size_t adjust_stack_size(size_t oc, size_t nc, size_t mc, ResizeAlgo algo);
size_t adjust_table_size(double lf, size_t nc, size_t mc);

#endif
