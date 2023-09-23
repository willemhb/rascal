#ifndef rascal_memory_h
#define rascal_memory_h

#include "common.h"
#include "object.h"

// C types
typedef struct {
  Obj*    objects;
  size_t  heapUsed;
  size_t  heapCapacity;
  Objects grays;
} Heap;

#define SAFE_ALLOC(func, args...)                           \
  ({                                                        \
    void* __out = func(args);                               \
                                                            \
    if (__out == NULL) {                                    \
      fprintf(stderr,                                       \
              "Out of memory calling %s at %s:%s:%d.\n",    \
              #func,                                        \
              __FILE__,                                     \
              __func__,                                     \
              __LINE__);                                    \
      exit(1);                                              \
    }                                                       \
    __out;                                                  \
  })

#define SAFE_MALLOC(nBytes)           SAFE_ALLOC(malloc, nBytes)
#define SAFE_REALLOC(pointer, nBytes) SAFE_ALLOC(realloc, pointer, nBytes)

void* allocate(size_t nBytes, bool fromHeap);
void* duplicate(void* pointer, size_t nBytes, bool fromHeap);
void* reallocate(void* pointer, size_t oldSize, size_t newSize, bool fromHeap);
void  deallocate(void* pointer, size_t nBytes, bool fromHeap);

#endif
