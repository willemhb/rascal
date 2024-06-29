#ifndef rl_util_table_h
#define rl_util_table_h

#include "common.h"

/* Utilities for working with dynamically sizable collections (tables and dynamic arrays). */
/* C types */
typedef enum {
  RESIZE_DEFAULT= 0, // use contextual fallback
  RESIZE_NONE   = 1, // don't allow resize
  RESIZE_STACK  = 2, // power of 2
  RESIZE_PYTHON = 3, // Python list algorithm (still amortized constant but preserves space)
  RESIZE_EXACT  = 4, // don't pad
} ResizeAlgo;

// collection utilities
size_t adjust_buffer_size(size_t oc, size_t nc, size_t mc, ResizeAlgo algo);
size_t adjust_stack_size(size_t oc, size_t nc, size_t mc, ResizeAlgo algo);
size_t adjust_table_size(double lf, size_t nc, size_t mc);

#endif
