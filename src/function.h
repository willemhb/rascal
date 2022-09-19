#ifndef rascal_function_h
#define rascal_function_h

#include "object.h"
#include "vector.h"
#include "binary.h"
#include "table.h"

// C types --------------------------------------------------------------------
typedef struct
{
  UInt16          : 12;
  UInt16 native   :  1;
  UInt16 bytecode :  1;
  UInt16 vargs    :  1;
} FunctionFl;

struct Function
{
  OBJ_HEAD(FunctionFl);

  String *name;

  union
  {
    NativeFn  native;
    ByteCode *bytecode;
  };
};

struct Closure
{
  OBJ_HEAD(UInt16);

  Function  *function;
  ArrayList *upvalues;
};

// utility macros -------------------------------------------------------------
#define fnArgco(val)     (asFunction(val)->object.arity)
#define fnIsNative(val)  (asFunction(val)->object.flags.native)
#define fnIsCode(val)    (asFunction(val)->object.flags.bytecode)
#define fnName(val)      (asFunction(val)->name)
#define fnNative(val)    (asFunction(val)->native)
#define fnByteCode(val)  (asFunction(val)->bytecode)

#define clNStack(val)    (asClosure(val)->object.arity)
#define clFunction(val)  (asClosure(val)->function)
#define clUpvalues(val)  (asClosure(val)->upvalues)

#endif
