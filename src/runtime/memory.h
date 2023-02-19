#ifndef runtime_memory_h
#define runtime_memory_h

#include "types/object.h"

/* C types */
struct Heap {
  usize allocated;
  usize available;
  Obj*  live;
  Obj** grays;
  int   count, cap;
};

/* globals */
extern struct Heap Heap;

/* API */
void *allocate(usize n, ubyte ini);
void *callocate(usize n, usize obsize, word ini);
void *reallocate(void* ptr, usize newn, usize oldn, ubyte ini);
void *creallocate(void* ptr, usize newn, usize oldn, usize obsize, word ini);
void *duplicate(void* ptr, usize n);
void  deallocate(void* ptr, usize n);
void  cdeallocate(void* ptr, usize n, usize obsize);
void  add_gray(void* obj);

#endif
