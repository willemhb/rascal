#ifndef rascal_memory_h
#define rascal_memory_h

#include <stdio.h>

// memory api, eventual GC implementation
void *allocate( size_t n_bytes );
void *alloc_array( size_t count, size_t ob_size );
void *reallocate( void *ptr, size_t n_bytes );
void *realloc_array( void *ptr, size_t count, size_t ob_size );
void  deallocate( void *ptr, size_t size );
void  dealloc_array( void *ptr, size_t count, size_t ob_size );
void  init_memory( void *ptr, size_t n_bytes );
void  init_array_memory( void *ptr, size_t count, size_t ob_size );

#define alloc_s( func, ... )						\
  ({									\
    Void *__out__ = func(__VA_ARGS__);					\
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
