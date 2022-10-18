#ifndef rascal_function_h
#define rascal_function_h

#include "obj/object.h"
#include "obj/record.h"
#include "obj/array.h"

struct function_t
{
  object_t        object;

  symbol_t       *name;

  value_t         method;
  type_t         *type;

  function_t     *next;
  function_t     *cache;

  ushort          vargs;
  ushort          final;
  arity_t         arity;
  type_t        **signature;
};

struct opcodes_t
{
  ARRAY_SLOTS(opcode_t, 2);
};

struct constants_t
{
  TABLE_SLOTS(entry_t*);
};

struct bytecode_t
{
  record_t record;

  arity_t  nArgs;
  arity_t  nStack;

  // execution state
  opcodes_t   *instructions;
  constants_t *constants;
  namespace_t *namespace;
  function_t  *function;
};

#endif
