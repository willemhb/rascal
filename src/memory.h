#ifndef memory_h
#define memory_h

#include "object.h"

/* C types */
struct Heap {
  usize  allocated;
  usize  available;
  Obj   *live;
  Vec   *grays;
};

/* globals */
extern struct Heap Heap;

/* API */
void *allocate(usize n_bytes);
void *reallocate(void *ptr, usize new_n_bytes, usize old_n_bytes);
void *duplicate(void *ptr, usize n_bytes);
void  deallocate(void *ptr, usize n_bytes);

#endif
