#ifndef rl_util_memory_h
#define rl_util_memory_h

#include <stdlib.h>

/* utilities for allocating and deallocating memory. */

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

#define SAFE_MALLOC(n_bytes)           SAFE_ALLOC(malloc, n_bytes)
#define SAFE_REALLOC(pointer, n_bytes) SAFE_ALLOC(realloc, pointer, n_bytes)

#endif
