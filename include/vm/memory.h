#ifndef rl_vm_memory_h
#define rl_vm_memory_h

#include "rascal.h"

/* commentary */

/* C types */

/* API */
void *alloc( size_t n_bytes );
void *alloc_array( size_t count, size_t ob_size );
void *alloc_cstring( size_t count, size_t ob_size );
void *dup( void *ptr, size_t n_bytes );
void *dup_array( void *ptr, size_t count, size_t ob_size );
void *dup_cstring( void *ptr, size_t count, size_t ob_size );
void *resize( void *ptr, size_t n_old, size_t n_new );
void *resize_array( void *ptr, size_t n_old, size_t n_new, size_t ob_size );
void *resize_cstring( void *ptr, size_t n_old, size_t n_new, size_t ob_size );
void  dealloc( void *ptr, size_t size );
void  dealloc_array( void *ptr, size_t count, size_t ob_size );
void  dealloc_cstring( void *ptr, size_t count, size_t ob_size );
void  init_memory( void *ptr, size_t n_bytes );
void  init_array_memory( void *ptr, size_t count, size_t ob_size );
void  init_cstring_memory( void *ptr, size_t count, size_t ob_size );

void collect_garbage( void );

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
