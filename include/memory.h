#ifndef rascal_memory_h
#define rascal_memory_h

#include <stdio.h>

#include "value.h"

// memory api, eventual GC implementation
Void *allocate( Size nBytes );
Void *allocArray( Size count, Size obSize );
Void *reallocate( Void *ptr, Size nBytes );
Void *reallocArray( Void *ptr, Size count, Size obSize );
Int   deallocate( Void *ptr, Size size );
Int   deallocArray( Void *ptr, Size count, Size obSize );
Void  initMemory( Void *ptr, Size nBytes );
Void  initArrayMemory( Void *ptr, Size count, Size obSize );

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
