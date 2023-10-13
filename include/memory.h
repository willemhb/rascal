#ifndef rascal_memory_h
#define rascal_memory_h

#include "common.h"
#include "object.h"

// C types
struct Heap {
  Obj*    objects;
  size_t  used;
  size_t  capacity;
  Objects grays;
  Values  saved;
};

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

void  initHeap(Heap* heap);
void  freeHeap(Heap* heap);

void  save(Value value);
void  unsave(size_t n);

void* allocate(Vm* vm, size_t nBytes);
void* duplicate(Vm* vm, void* pointer, size_t nBytes);
void* reallocate(Vm* vm, void* pointer, size_t oldSize, size_t newSize);
void  deallocate(Vm* vm, void* pointer, size_t nBytes);

#endif
