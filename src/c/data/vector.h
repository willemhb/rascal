#ifndef data_alist_h
#define data_alist_h

#include "data/object.h"

// C types
struct alist {
  HEADER;
  ARRAY(value_t);
};

// APIs & utilities
#define   as_alist(x) ((alist_t*)((x) & WVMASK))

alist_t*  alist(usize n);
void      reset_alist(alist_t* slf, usize n);
usize     alist_push(alist_t* slf, value_t val);
value_t   alist_pop(alist_t* slf);

#endif
