#ifndef rl_util_memory_h
#define rl_util_memory_h

#include "common.h"

/* commentary

   mostly just safe versions of stdlib.h functions. */

/* C types */

/* globals */

/* API */
void *malloc_s( size_t n_bytes );
void *calloc_s( size_t count, size_t ob_size );
void *oballoc_s( size_t base, size_t count, size_t ob_size );
void *realloc_s( void *ptr, size_t new_size );
void *crealloc_s( void *ptr, size_t new_count, size_t ob_size );
void *obrealloc_s( void *ptr, size_t base, size_t count, size_t ob_size );

/* runtime */

/* convenience */
#define alloc_s( func, ... )						\
  ({									\
    void *__out__ = func(__VA_ARGS__);					\
									\
    if ( __out__ == NULL )						\
      {									\
	fprintf( stderr,						\
		 "Allocation failed at %s:%d in %s calling %s.\n",	\
		 __FILE__,						\
		 __LINE__,						\
		 __func__,						\
		 #func );						\
	abort();							\
      }									\
    __out__;								\
  })

#endif
