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
void *safeMalloc( const char *fname, size_t nBytes );
void *safeCalloc( const char *fname, size_t count, size_t obSize );
void *safeRealloc( const char *fname, void *ptr, size_t nBytes );
void  safeFree( const char *fname, void *ptr );
void *memDup( void *ptr, size_t nBytes );

// rascal memory API ----------------------------------------------------------
Obj   *allocateObject(size_t size, ObjType type);
Obj   *duplicateObject(Obj *obj, Obj **location );
Value  duplicateValue( Value val, Value *location );
void  *reallocate( void* pointer, Obj **cache, size_t oldCnt, size_t newSize );
void   markObject( Obj* object );
void   markValue( Value value );
void   collectGarbage( void );
void   freeObjects( void );

// utility macros -------------------------------------------------------------
#define ALLOCATE_OBJ(type, objectType)			\
    (type*)allocateObject(sizeof(type), objectType)

#define ALLOCATE(type, count)					\
  (type*)reallocate(NULL, NULL, 0, sizeof(type) * (count))


#define DUP_ARRAY(type, src, arrLen, arrSize)		\
  ({							\
    type*_out_ = ALLOCATE( type, arrSize );		\
    memcpy( _out_, src, arrLen * sizeof(type));		\
    _out_;						\
  })

#define DUP_OBJECT(obType, src)						\
  ({									\
    obType *_src_ = src;						\
    obType *_out_ = ALLOCATE_OBJ( obType, src->object.type );		\
    memcpy((char*)(&_out_->object.arity),				\
	   (char*)(&_src_->object.arity),				\
	   sizeof(obType)-8 );						\
    _out_;								\
  })

#define FREE(type, pointer) reallocate(pointer, sizeof(type), 0)

#define GROW_CAPACITY(capacity)			\
  ((capacity) < 8 ? 8 : (capacity) * 2)

#define GROW_ARRAY(type, cache, pointer, oldCount, newCount)	\
  (type*)reallocate(pointer, cache, sizeof(type) * (oldCount),	\
        sizeof(type) * (newCount))

#define FREE_ARRAY(type, pointer, oldCount)		\
    reallocate(pointer, sizeof(type) * (oldCount), 0)

#define SET_GRAY(obj)    ((obj)->next |= GRAY)
#define CLEAR_GRAY(obj)  ((obj)->next &= ~GRAY)
#define SET_BLACK(obj)   ((obj)->next |= BLACK)
#define CLEAR_BLACK(obj) ((obj)->next &= ~BLACK)
#define IS_GRAY(obj)     (((obj)->next & GRAY) == GRAY)
#define IS_BLACK(obj)    (((obj)->next & BLACK) == BLACK)
#define IS_LIGHT(obj)    (((obj)->next & LIGHT) == LIGHT)
#define IS_DARK(obj)     (((obj)->next & DARK) == DARK)

#endif
