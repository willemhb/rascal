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

#define ARRAY_API(arrayType, elType)				\
  void   init##arrayType( arrayType *array );			\
  void   finalize##arrayType( arrayType *array );		\
  void   write##arrayType( arrayType *array, elType element )


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

// utility macros & statics ---------------------------------------------------


#endif
