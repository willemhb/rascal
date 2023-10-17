#ifndef rascal_util_collection_h
#define rascal_util_collection_h

#include "common.h"

/* general collection utilities */
bool   check_alist_resize(size_t new_cnt, size_t old_cap, bool encoded);
bool   check_table_resize(size_t new_cnt, size_t old_cap);

size_t pad_alist_size(size_t old_cnt, size_t new_cnt, size_t old_cap, bool encoded);
size_t pad_stack_size(size_t old_cnt, size_t new_cnt, size_t old_cap, bool encoded);
size_t pad_ord_size(size_t old_cnt, size_t new_cnt, size_t old_cap);
size_t get_ord_size(size_t cnt);
void*  alloc_ords(size_t cap, size_t o_size);

#endif
