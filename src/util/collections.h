#ifndef util_collections_h
#define util_collections_h

#include "common.h"

typedef uintptr_t Val;

/* API */
// size padding ---------------------------------------------------------------
usize pad_buffer_size(usize count, usize cap);
usize pad_alist_size(usize count, usize cap);
usize pad_table_size(usize count, usize cap);

// accessors & mutators for dispatching on C type -----------------------------
Val   ascii_ref(void* data, int i);
Val   ascii_set(void* data, int i, int max, Val x);
usize ascii_write(void* buf, usize n, void* data);

Val   uint16_ref(void* data, int i);
Val   uint16_set(void* data, int i, int max, Val x);
usize uint16_write(void* buf, usize n, void* data);


#endif

