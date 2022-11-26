#ifndef rl_vm_memory_h
#define rl_vm_memory_h

#include "rascal.h"

/* commentary */

/* C types */

/* globals */

/* API */
void *alloc( size_t n_bytes );
void *alloc_array( size_t count, size_t ob_size );

void  dealloc( void *ptr, size_t n_bytes );
void  dealloc_array( void *ptr, size_t count, size_t ob_size );

void *adjust( void *ptr, size_t old_size, size_t new_size );
void *adjust_array( void *ptr, size_t old_count, size_t new_count, size_t ob_size );

/* runtime */
void rl_vm_memory_init( void );

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
