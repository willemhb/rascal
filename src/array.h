#ifndef rascal_array_h
#define rascal_array_h

#include "value.h"

#define ARRAY_SPEC(elType)			\
  int     length;				\
  int     capacity;				\
  elType *data

#define ARRAY_API(arrayType, elType)			\
  void init##arrayType( arrayType *array );		\
  void write##arrayType( arrayType *array, elType el ); \
  void finalize##arrayType( arrayType *array );		\
  void free##arrayType( arrayType *array )


typedef struct
{
  ARRAY_SPEC(Value);
} ValueArray;

typedef struct
{
  ARRAY_SPEC(uint16_t);
} UInt16Array;

typedef struct
{
  ARRAY_SPEC(int32_t);
} Int32Array;

// forward declarations -------------------------------------------------------
ARRAY_API( ValueArray, Value );
ARRAY_API( UInt16Array, uint16_t );
ARRAY_API( Int32Array, int32_t );

#endif
