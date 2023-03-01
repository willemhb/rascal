#ifndef memory_h
#define memory_h

#include "common.h"

// API ------------------------------------------------------------------------
#define ALLOC_S(func, args...)                  \
  ({                                            \
    void* __out = func(args);                   \
    if (__out == NULL)                          \
      FAIL(1, "in %s: out of memory", #func);	\
    __out;                                      \
  })

void* allocate(usize size);
void* duplicate(void* ptr, usize size);
void* reallocate(void* ptr, usize old, usize new);
void  deallocate(void* ptr, usize size);
void  manage(void);

// initialization -------------------------------------------------------------
void memory_init(void);

#endif
