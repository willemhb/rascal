#include <stdlib.h>

#include "memory.h"
#include "array.h"

#define ARRAY_IMPL( arrayType, elType )					\
  void init##arrayType( arrayType *array )				\
  {									\
    array->length   = 0;						\
    array->capacity = 0;						\
    array->data     = NULL;						\
  }									\
  void write##arrayType( arrayType *array, elType el )			\
  {									\
    if ( array->capacity < array->length + 1 )				\
      {									\
	int oldCapacity = array->capacity;				\
	array->capacity = GROW_CAPACITY( oldCapacity );			\
	array->data     = GROW_ARRAY( elType, array->data,		\
				      oldCapacity, array->capacity );	\
      }									\
    array->data[array->length] = el;					\
    array->length++;							\
  }									\
  void free##arrayType( arrayType *array )				\
  {									\
    FREE_ARRAY( elType, array->data, array->capacity );			\
    init##arrayType( array );						\
  }									\
  void finalize##arrayType( arrayType *array )				\
  {									\
    free##arrayType( array );						\
    free( array );							\
  }

ARRAY_IMPL( ValueArray, Value )
ARRAY_IMPL( UInt16Array, uint16_t )
ARRAY_IMPL( Int32Array, int32_t )
