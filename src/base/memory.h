#ifndef rascal_base_memory_h
#define rascal_base_memory_h

#include "base/object.h"

/* C types */
struct Heap {
  Object *live;
  Objects grays;
  usize   allocated;
  usize   nextgc;
};

/* API */
void *allocate(usize size);
void *resize(void *ptr, usize oldsize, usize newsize);
void *deallocate(void *ptr, usize size);

#endif
