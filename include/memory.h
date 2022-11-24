#ifndef rascal_memory_h
#define rascal_memory_h

#include <stdio.h>
#include "rascal.h"

/* memory & GC */
/* internal objects */
typedef struct heap_t
{
  vector_t *gray_objects;
  object_t *live_objects;
  size_t    bytes_allocated;
  size_t    next_cycle;
} heap_t;

/* globals */
extern heap_t Heap;

/* API */
void *alloc( size_t n_bytes );
void *array_alloc( size_t count, size_t ob_size );
void *resize( void *ptr, size_t n_old, size_t n_new );
void *array_resize( void *ptr, size_t n_old, size_t n_new, size_t ob_size );
void  dealloc( void *ptr, size_t size );
void  array_dealloc( void *ptr, size_t count, size_t ob_size );
void  init_memory( void *ptr, size_t n_bytes );
void  init_array_memory( void *ptr, size_t count, size_t ob_size );

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
