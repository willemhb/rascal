#ifndef rascal_func_h
#define rascal_func_h

#include "obj.h"

typedef struct func_t
{
  OBJ_HEAD;

  obj_t *name;
  obj_t *tplt;
} func_t;

typedef struct native_t
{
  OBJ_HEAD;

  val_t (*native)(val_t *args, arity_t n_args);
} native_t;

typedef struct code_t
{
  OBJ_HEAD;
  
  obj_t *code;
  obj_t *vals;
} code_t;

typedef struct closure_t
{
  OBJ_HEAD;

  obj_t *func;
  obj_t *envt;
} closure_t;

#endif
