#ifndef rascal_method_h
#define rascal_method_h

#include "table.h"

struct metht_t
{
  obj_t obj;
  TABLE_SLOTS(method_t*);

  arity_t min_argc;
  arity_t max_argc;
  func_t   *func;
  mlevel_t *tree;
};

struct mlevel_t
{
  obj_t obj;
  TABLE_SLOTS(mlevel_t*);
  ENTRY_SLOTS(type_t, type, method_t *, method);
  arity_t pos;
};

struct method_t
{
  obj_t    obj;
  ENTRY_SLOTS(types_t*, signature, union { native_fn_t native; }, method );
  
};


#endif
