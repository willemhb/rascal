#ifndef runtime_memory_h
#define runtime_memory_h

#include  "common.h"

// API ------------------------------------------------------------------------
#define ALLOC_S(func, args...)                  \
  ({                                            \
    void* __out = func(args);                   \
    if (__out == NULL)                          \
      FAIL(1, "in %s: out of memory", #func);	\
    __out;                                      \
  })

void* allocate(usize size);
void* duplicate(void* ptr, usize size, usize pad);
void* reallocate(void* ptr, usize old, usize new);
void  deallocate(void* ptr, usize size);
void  push_gray(void* obj);
void  manage(void);

// initialization -------------------------------------------------------------
void memory_init(void);

#endif
