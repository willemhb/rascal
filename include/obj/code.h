#ifndef rascal_code_h
#define rascal_code_h

#include "obj.h"


DECL_OBJ(code);
DECL_OBJ_API(code);

struct code_t
{
  OBJ_HEAD;
  arity_t n_local;
  arity_t n_closure;

  obj_t *code;
  obj_t *vals;
};


#endif
