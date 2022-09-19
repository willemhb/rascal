#ifndef rascal_memory_h
#define rascal_memory_h

#include "object.h"

// C types --------------------------------------------------------------------
typedef Void  (*TraceFn)(Obj *object);
typedef Void  (*FinalizeFn)(Obj *object);
typedef Size  (*SizeOfFn)(Obj *object);
typedef Obj  *(*ConstructFn)(Size count, Flags flags);
typedef Obj  *(*ResizeFn)( Obj *object, Size new );
typedef Void  (*InitializeFn)( Obj *object, Size count, Void *data, Flags flags );

typedef enum
  {
    ALLOC_ALLOCATE   = 0x0100, // object should be allocated
    ALLOC_SHARED     = 0x0200, // object shares data with a parent
    ALLOC_INLINED    = 0x0400, // object data is stored directly
    ALLOC_COPY_INI   = 0x0800, // initialize from identical object
    ALLOC_PRIM_INI   = 0x1000, // initialize from untagged primitive data
    ALLOC_STACK_INI  = 0x2000, // initializers passed as data on stack (Native call)
    ALLOC_SPECIAL_INI= 0x4000, // initializers passed from a special data structure
  } AllocFl;

// allocation flag predicates -------------------------------------------------
static inline FlagPredicate(Alloc, ALLOC_ALLOCATE)
static inline FlagPredicate(Shared, ALLOC_SHARED)
static inline FlagPredicate(CopyIni, ALLOC_COPY_INI)
static inline FlagPredicate(PrimIni, ALLOC_PRIM_INI)
static inline FlagPredicate(StackIni, ALLOC_STACK_INI)
static inline FlagPredicate(SpecialIni, ALLOC_SPECIAL_INI)

// globals --------------------------------------------------------------------
extern TraceFn      TraceDispatch[NUM_TYPES];
extern FinalizeFn   FinalizeDispatch[NUM_TYPES];
extern Size         BaseSizeDispatch[NUM_TYPES];
extern SizeOfFn     SizeOfDispatch[NUM_TYPES];
extern ConstructFn  ConstructDispatch[NUM_TYPES];
extern ResizeFn     ResizeDispatch[NUM_TYPES];
extern InitializeFn InitializeDispatch[NUM_TYPES];

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

// utilities for initializing arrays & tables ---------------------------------
#define initArray(array, count)    memset( array, 0, count * sizeof(typeof(array[0])) )
#define initDispatch(Table)        initArray( Table, NUM_TYPES )

// toplevel constructor --------------------------------------------------------
Obj   *constructObj( ValueType type, Size count, Void *data, Flags fl );

// toplevel GC -----------------------------------------------------------------
Void   collectGarbage( Void );
Void   freeObjects( Void );

// allocate --------------------------------------------------------------------
#define sizeOf(arg)				\
  _Generic((arg),				\
	   Value:valSizeOf,			\
	   Obj*objSizeOf)(arg)

Size valSizeOf( Value val );
Size objSizeOf( Obj *obj );

#define allocate(arg, ...)						\
  _Generic((arg),							\
	   Size: allocateBytes,						\
	   Arity: allocateBytes,					\
	   ValueType: allocateObject,					\
	   Int: allocateObject)((arg) __VA_OPT__(,) __VA_ARGS__)

Void  *allocateBytes( Size nBytes );
Obj   *allocateObject( ValueType objType, Size count, Void *data, Flags fl );

#define reallocate(arg, ...)					\
  _Generic((arg),						\
	   Void*:reallocateBytes,				\
	   Value:realocateVal,					\
	   Obj*:reallocateObj)((arg) __VA_OPT__(,) __VA_ARGS__)
  
Void  *reallocateBytes( Void *ptr, Size old, Size new );
Value  reallocateVal( Value val, Size new );
Obj   *reallocateObj( Obj *obj, Size new );

// mark ------------------------------------------------------------------------
#define mark(val)				\
  _Generic((val),				\
	   Value:markValue,			\
	   Obj*:markObject			\
	   )(val)

Void markObject( Obj *object );
Void markValue( Value val );

// duplicate -------------------------------------------------------------------
#define duplicate(val, ...)						\
  _Generic((val),							\
	   Value:   duplicateValue,					\
	   Obj*:    duplicateObject,					\
	   default: duplicateMemory )((val) __VA_OPT__(,) __VA_ARGS__ )

Void  *duplicateMemory( Void *ptr, Size nBytes );
Obj   *duplicateObject(Obj *obj );
Value  duplicateValue( Value val );

// finalize -------------------------------------------------------------------
#define finalize(val, ...)						\
  _Generic((val),							\
	   Value: finalizeValue,					\
	   Obj*:  finalizeObject,					\
	   Value*:finalizeValues,					\
	   Obj**:finalizeObjects)((val) __VA_OPT__(,) __VA_ARGS__)

Void finalizeValue( Value val );
Void finalizeObject( Obj *obj );
Void finalizeValues( Value *values, Arity count );
Void finalizeObjects( Obj** obj, Arity count );

// trace ----------------------------------------------------------------------
#define trace(val, ...)						\
  _Generic((val),						\
	   Value: traceValue,					\
	   Obj*: traceObject,					\
	   Value*:traceValues,					\
	   Obj**:traceObjects)((val) __VA_OPT__(,) __VA_ARGS__)

Void   traceValue( Value val );
Void   traceObject( Obj *obj );
Void   traceValues( Value *vals, Arity count );
Void   traceObjects( Obj **objs, Arity count );

// initialization -------------------------------------------------------------

#endif
