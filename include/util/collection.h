#ifndef rl_util_collection_h
#define rl_util_collection_h

#include "common.h"

/* general collection utilities */
bool   check_alist_resize(size_t new_cnt, size_t old_cap, bool encoded);
bool   check_table_resize(size_t new_cnt, size_t old_cap);

size_t pad_alist_size(size_t old_cnt, size_t new_cnt, size_t old_cap, bool encoded);
size_t pad_stack_size(size_t old_cnt, size_t new_cnt, size_t old_cap, bool encoded);
size_t pad_table_size(size_t old_cnt, size_t new_cnt, size_t old_cap, double loadf);

#endif
