#ifndef rascal_function_h
#define rascal_function_h

#include "table.h"

typedef enum
  {
    method_fl_primitive=0x0,
    method_fl_native   =0x1,
    method_fl_bytecode =0x2,
    method_fl_script   =0x3,
  } method_fl_t;

typedef union
{
  opcode_t     primitive;
  native_fn_t  native;
  bytecode_t  *bytecode;
  bytecode_t  *script;
} invoke_t;

struct function_t
{
  object_t        object;

  symbol_t       *name;
  type_t         *type;
  method_table_t *methods;
};

struct method_table_t
{
  object_t       object;

  function_t    *function;
  method_t      *fargs;
  method_t      *vargs;
  method_t      *cache;
};

struct method_t
{
  object_t    object;

  method_t       *cache;
  method_t       *next;
  function_t     *function;
  method_table_t *table;
  invoke_t        invoke;
  size_t          arity;
  type_t         *return_type;
  type_t        **argument_types;
};

// globals
extern type_t *FunctionType, *MethodType;

// forward declarations
function_t *new_function(void);
void        init_function(function_t *function, symbol_t *name);

bool        get_method(function_t *function, type_t *input_types, arity_t n, method_t **buf);
bool        put_method(function_t *function, type_t *input_types, arity_t n, method_t **buf);

method_t   *new_method(arity_t n);
void        init_method(method_t *method, function_t *function, invoke_t invoke, type_t return_type, arity_t n, type_t *argument_types);



#endif
