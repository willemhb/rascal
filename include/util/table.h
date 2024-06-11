#ifndef rl_util_table_h
#define rl_util_table_h

#include "common.h"

/* Utilities for working with dynamically sizable collections (tables and dynamic arrays). */
// collection utilities
size_t adjust_buffer_size(size_t oc, size_t nc, size_t mc, bool p);
size_t adjust_stack_size(size_t oc, size_t nc, size_t mc, bool p);
size_t adjust_table_size(double lf, size_t nc, size_t mc);

#endif
