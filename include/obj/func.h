#ifndef rascal_func_h
#define rascal_func_h

#include "obj.h"

typedef struct func_t func_t;

DECL_OBJ(func);

struct func_t
{
  OBJ_HEAD;

  bool    vargs;
  arity_t argco;

  obj_t *name;
  obj_t *ns;
  obj_t *tplt;
};

#endif
