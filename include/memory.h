#ifndef rascal_memory_h
#define rascal_memory_h

#include "common.h"
#include "object.h"

// globals
#define N_HEAP  (((size_t)1)<<19)
#define HEAP_LF 0.625

// external API
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

#define mark(x) generic2(mark, x, x)

void  markVal(Value val);
void  markObj(void* ptr);

void  initHeap(Vm* vm);
void  freeHeap(Vm* vm);

void  addToHeap(void* p);
void* allocate(Vm* vm, size_t nBytes);
void* duplicate(Vm* vm, void* pointer, size_t nBytes);
char* duplicates(Vm* vm, char* chars, size_t nChars);
void* reallocate(Vm* vm, void* pointer, size_t oldSize, size_t newSize);
void  deallocate(Vm* vm, void* pointer, size_t nBytes);

#endif
