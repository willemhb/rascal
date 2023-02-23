#ifndef util_collections_h
#define util_collections_h

#include "common.h"

// globals --------------------------------------------------------------------
#define MinSCap 8ul
#define MinTCap 8ul
#define MinACap 1ul
#define LoadF   0.625l

// size padding ---------------------------------------------------------------
usize pad_stack_size(usize count, usize cap);
usize pad_alist_size(usize oldct, usize newct, usize oldcap);
usize pad_table_size(usize count, usize cap);

#endif

