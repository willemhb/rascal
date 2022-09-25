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

  obj_t  *name;
  obj_t  *ns;
  obj_t  *tplt;
  obj_t  *sig;
};

struct sig_t
{
  OBJ_HEAD;

  arity_t arity;
  bool    vargs;

  hash_t  hash;

  val_t   type;
  val_t   sig[1];
};

#endif
