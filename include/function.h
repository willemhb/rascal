#ifndef rascal_function_h
#define rascal_function_h

#include "object.h"


enum
  {
    primitive_method    =0,
    native_method       =1,
    closure_method      =2,
    script_method       =3,

    no_kind             =0, // not a type
    bottom_kind         =1, // special case of bottom type
    top_kind            =2, // special case of top type
    singleton_kind      =3, // special case of nul type
    primitive_kind      =4, // builtin type
    record_kind         =5, // product
    union_kind          =6,
  };

struct function_t
{
  object_t        object;

  symbol_t       *name;

  invoke_t        method;
  type_repr_t     type;

  function_t     *next;
  function_t     *cache;

  flags_t                    : 24;
  flags_t         final      :  1;
  flags_t         vargs      :  1;
  flags_t         typeKind   :  3;
  flags_t         methodType :  3;

  arity_t         arity;
  type_t         *returnType;
  type_t        **argTypes;
};

struct control_t
{
  object_t   object;

  arity_t     size;     // number of locals
  arity_t     base;     // offset of first argument

  alist_t    *eval;     // stack where locals live
  envt_t     *envt;
  bytecode_t *code;     // executing code object
  opcode_t   *prgc;     // program counter
  control_t  *cont;     // immediate caller
  control_t  *cntl;     // continuation prompt (last place where (with ...) form appears)
};

struct bytecode_t
{
  object_t     object;

  arity_t      nArgs;
  arity_t      nStack;

  // execution state
  buffer_t    *instructions;
  table_t     *constants;
  function_t  *function;
  ns_t        *ns;
};

#endif
