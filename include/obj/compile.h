#ifndef rascal_compile_h
#define rascal_compile_h

#include "obj/array.h"
#include "obj/table.h"

// bytecode object and constituents
typedef struct bytecode_t  bytecode_t;
typedef struct opcodes_t   opcodes_t;
typedef struct constants_t constants_t;

struct bytecode_t
{
  OBJECT;

  arity_t   n_args;
  arity_t   n_stack;
  
  object_t *function;
  object_t *code;
  object_t *constants;
  object_t *namespace;
};

// bytecode array
struct opcodes_t
{
  ARRAY(opcode_t);
};

// constants table
struct constants_t
{
  TABLE;
};

// globals
extern type_t BytecodeType, OpcodesType, ConstantsType;

#endif
