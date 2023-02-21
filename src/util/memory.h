#ifndef util_memory_h
#define util_memory_h

#include "common.h"

// API ------------------------------------------------------------------------
// initialization helpers -----------------------------------------------------
void* mem8set( void* dest, uint8 ch, usize count );
void* mem16set( void* dest, uint16 ch, usize count );
void* mem32set( void* dest, uint32 ch, usize count );
void* mem64set( void* dest, uint64 ch, usize count );
void* memxxset( void* dest, const void* src, usize count, usize obsize );

// safe allocators ------------------------------------------------------------
void* malloc_s( usize n );
void* calloc_s( usize n, usize obsize );
void* realloc_s( void* p, usize n );
void* crealloc_s( void* p, usize n, usize obsize );

#define SAFE_ALLOC(func, args...)					\
  ({									\
    void* __out = func(args);						\
    if (__out == NULL) {						\
      fprintf(stderr, "Allocation failed in "#func".\n");		\
      exit(1);								\
    }									\
    __out;								\
  })

#endif
