#ifndef rl_util_collection_h
#define rl_util_collection_h

#include "common.h"

/* commentary

   Collection implementation macros typically take a size function as a parameter. Those
   functions are defined here. */

/* C types */
typedef size_t (*padfn_t)(size_t new_count, size_t old_count, size_t old_cap);

/* globals */

/* API */

/* runtime */

/* convenience */
size_t pad_array_size( size_t new_count, size_t old_count, size_t old_cap );
size_t pad_string_size( size_t new_count, size_t old_count, size_t old_cap );
size_t pad_stack_size( size_t new_count, size_t old_count, size_t old_cap );
size_t pad_alist_size( size_t new_count, size_t old_count, size_t old_cap );
size_t pad_buffer_size( size_t new_count, size_t old_count, size_t old_cap );
size_t pad_table_size( size_t new_count, size_t old_count, size_t old_cap );

#endif
