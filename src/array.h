#ifndef rascal_array_h
#define rascal_array_h

#include "object.h"

#define ARRAY_SPEC(elType)				\
  int     length;					\
  int     capacity;					\
  union							\
  {							\
    elType  space[0];					\
    elType *data;					\
  }

#define ARRAY_API(arrayType, elType)				          \
  arrayType *new##arrayType(size_t count);				  \
  void       init##arrayType(arrayType *array,size_t count,elType *data); \
  void       finalize##arrayType(arrayType *array);			  \
  void       write##arrayType(arrayType *array,elType element)

struct Tuple
{
  ARRAY_SPEC(Value);
};

struct String
{
  ARRAY_SPEC(Char);
  hash_t hash;
};

struct Bytecode
{
  ARRAY_SPEC(Instruction);
};

// forward declarations -------------------------------------------------------
ARRAY_API(Tuple, Value);
ARRAY_API(String, Char);
ARRAY_API(Bytecode, Instruction);

// utility macros & statics ---------------------------------------------------
#define IS_TUPLE(val) (isObjType(val, OBJ_TUPLE))
#define IS_

#endif
