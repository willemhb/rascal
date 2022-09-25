#ifndef rascal_func_h
#define rascal_func_h

#include "obj.h"

typedef struct func_t func_t;
typedef struct repr_t repr_t;
typedef struct str_t  str_t;

DECL_OBJ(func);
DECL_OBJ_API(func);

struct func_t
{
  OBJ_HEAD;

  bool    vargs;
  arity_t argco;

  obj_t *type;    // the type this function is a constructor for (if any)
  obj_t *sig;     // the function's signature

  str_t *name;
  obj_t *ns;
  obj_t *tplt;
};

#endif
