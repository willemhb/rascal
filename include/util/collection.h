#ifndef rl_util_collection_h
#define rl_util_collection_h

#include "common.h"

/* commentary

   general utilities for working with collections (arrays, alists, tables, &c) go here now. */

/* C types */

/* globals */

/* API */

/* runtime */

/* convenience */
size_t pad_py_alist_size( size_t new_count, size_t old_count, size_t old_cap );
size_t pad_buffer_size( size_t new_count, size_t old_cap );
size_t pad_alist_size( size_t new_count, size_t old_cap );
size_t pad_table_size( size_t new_count, size_t old_cap );

#endif
