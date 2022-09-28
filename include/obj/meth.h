#ifndef rascal_meth_h
#define rascal_meth_h

#include "obj.h"

#include "template/str.h"
#include "template/arr.h"
#include "template/table.h"

typedef struct meth_t   meth_t;
typedef struct metht_t  metht_t;
typedef struct func_t   func_t;
typedef struct code_t   code_t;

DECL_STRING(sig);
DECL_TABLE(mcache);
DECL_TABLE(mlevel);

STRING_TYPE(sig, EMPTY_HEAD, type_t, type_t, type);
DECL_STRING_API(sig, type_t, type_t, type);

TABLE_TYPE(mcache, OBJ_HEAD, meth_t*);
DECL_TABLE_API(mcache, sig_t*, meth_t*);

TABLE_TYPE(mlevel, OBJ_HEAD, meth_t*);
DECL_TABLE_API(mlevel, type_t, meth_t*);

struct metht_t
{
  /* invasive hash table of exact method signatures */
  mcache_t cache;

  /* min and max arities used when computing the signature hash */
  arity_t  argmin;
  arity_t  argmax;

  /* the function that owns this method table */ 
  func_t  *func;
  meth_t  *tree;
};

struct meth_t
{
  /* invasive hash table of */
  mlevel_t     children;
  sig_t        sig;

  union
  {
    opcode_t     primitive;
    native_fn_t  native;
    code_t      *bytecode;
  };

  metht_t       *ancestor;
  meth_t        *parent;
};

typedef enum
  {
    meth_fl_none     = 0x000, // no method matching the current signature
    meth_fl_primitive= 0x001,
    meth_fl_native   = 0x002,
    meth_fl_bytecode = 0x003,
    meth_fl_vargs    = 0x004
  } meth_fl_t;

// forward declarations
bool get_method(func_t *func, arity_t n, val_t *args, meth_t **buf );
bool put_method(func_t *func, arity_t n, val_t *args, meth_t **buf );
bool pop_method(func_t *func, arity_t n, val_t *args, meth_t **buf );

// 
#define as_meth(val)   ((meth_t*)as_obj(val))
#define is_meth(val)   is_obj_type(val, meth_type)

#define as_metht(val)  ((metht_t*)as_obj(val))
#define is_metht(val)  is_obj_type(val, metht_type)

#define as_mcache(val) ((mcache_t*)as_obj(val))
#define as_mlevel(val) ((mlevel_t*)as_obj(val))

#endif
