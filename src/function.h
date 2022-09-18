#ifndef rascal_function_h
#define rascal_function_h

#include "object.h"
#include "vector.h"
#include "binary.h"
#include "table.h"

// C types --------------------------------------------------------------------
typedef enum
  {
   // stack manipulation ------------------------------------------------------
   OP_PUSH,
   OP_POP,

   // load & store instructions -----------------------------------------------
   OP_CONSTANT,
   OP_LOAD_GLOBAL,
   OP_STORE_GLOBAL,
   OP_LOAD_LOCAL,
   OP_STORE_LOCAL,
   OP_LOAD_UPVALUE,
   OP_STORE_UPVALUE,
   
   // control flow -----------------------------------------------------------
   OP_JUMP,
   OP_JUMP_TRUE,
   OP_JUMP_FALSE,
   OP_CALL,
   OP_RETURN,
  } OpCode;



struct Function
{
  OBJ_HEAD(UInt16);

  Binary *name;

  Arity   arity;
  Arity   flags  : 31;
  Arity   vargs  :  1;
  
  union
  {
    Native   *native;
    ByteCode *bytecode;
  };
};

struct Native
{
  OBJ_HEAD(BinaryFl);
  NativeFn function;
};

struct ByteCode
{
  OBJ_HEAD(BinaryFl);
  
  Arity         nUpvalues;
  Arity         nArgs;
  
  Instructions *code;
  ArrayList    *constants;
};

struct Closure
{
  OBJ_HEAD(UInt16);

  Function  *function;
  ArrayList *upvalues;
};

#endif
