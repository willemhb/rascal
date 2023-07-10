#ifndef util_memory_h
#define util_memory_h

#include <stdlib.h>

#include "common.h"

// external API +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define SAFE_ALLOC( f, ... )                                            \
  ({                                                                    \
    void* __out = f(__VA_ARGS__);                                       \
    if ( __out == NULL ) {                                              \
      fprintf(stderr, "fatal error: out of memory in %s.\n", __func__); \
      exit(1);                                                          \
    }                                                                   \
    __out;                                                              \
  })

#endif
