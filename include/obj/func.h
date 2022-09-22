#ifndef rascal_func_h
#define rascal_func_h

#include "obj.h"

struct func_t
{
  OBJ_HEAD;

  bool    vargs;
  arity_t argco;

  obj_t *name;
  obj_t *ns;
  obj_t *tplt;
};

// container for a builtin function -------------------------------------------
typedef struct native_t
{
  OBJ_HEAD;
  
  val_t (*native)(val_t *args, arity_t n);
} native_t;

// bytecode for a compiled function -------------------------------------------
typedef struct code_t
{
  OBJ_HEAD;

  obj_t *code;
  obj_t *vals;
} code_t;

// runtime function representation --------------------------------------------
typedef struct closure_t
{
  OBJ_HEAD;

  obj_t *func;
  obj_t *ns;
  obj_t *upvals;
  obj_t *toplevel;
} closure_t;

// globals --------------------------------------------------------------------

// forward declarations -------------------------------------------------------
// toplevel dispatch ----------------------------------------------------------
void func_mark( void );
void func_init( void );
#endif
