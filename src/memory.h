#ifndef rascal_memory_h
#define rascal_memory_h

#include "object.h"

// memory flags (stored in low bits of ((Obj*)x)->next
#define GRAY      0x0000000000000004ul
#define BLACK     0x0000000000000002ul
#define WHITE     0x0000000000000000ul
#define LIGHT     (GRAY|WHITE)
#define DARK      (GRAY|BLACK)

#define ALLOCATED 0x0000000000000001ul


// forward declarations -------------------------------------------------------
// safe API for stdlib allocators ---------------------------------------------
#define safeAllocate( func, args... )					\
  ({									\
    void *_ptr_ = func( args );						\
    if (_ptr_ == NULL)							\
      {									\
	fprintf( stderr,						\
		 "Allocation failed at %s:%d in %s. Exiting.\n",	\
		 __FILE__,						\
		 __LINE__,						\
		 __func__ );						\
	exit(1);							\
      }									\
    _ptr_;								\
  })

#define safeMalloc( nBytes )        safeAllocate( malloc, nBytes )
#define safeCalloc( count, obSize ) safeAllocate( calloc, count, obSize )
#define safeRealloc( ptr, nBytes )  safeAllocate( realloc, ptr, nBytes )

// rascal memory API ----------------------------------------------------------
Void  *allocate( Size nBytes );
Void  *duplicate( Void *ptr, Size nBytes );
Obj   *allocateObject( ValueType objType, Size count );
Obj   *duplicateObject(Obj *obj );
Value  duplicateValue( Value val );
Void  *reallocate( Void *ptr, Size old, Size new );
void   markObject( Obj* object );
void   markValue( Value value );
void   collectGarbage( void );
void   freeObjects( void );

// utility macros -------------------------------------------------------------

#endif
