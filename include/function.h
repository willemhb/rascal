#ifndef rascal_func_h
#define rascal_func_h

#include "object.h"

struct function_t
{
  object_t    object;

  function_t *next;
  function_t *cache;

  symbol_t   *name;

  union
  {
    opcode_t     op;
    native_fn_t  native;
    cons_t      *code;
    vector_t    *cntl;
  };

  value_t   typeInfo;

  flags_t   methodType;
  bool      hasVargs;
  arity_t   argc;
  type_t    signature[1];
};

#endif
