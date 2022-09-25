#ifndef rascal_native_h
#define rascal_native_h

#include "obj.h"

typedef struct native_t native_t;
typedef struct str_t    str_t;

typedef val_t (*thunk_t)(void);
typedef val_t (*unary_t)(val_t x);
typedef val_t (*binary_t)(val_t x, val_t y);
typedef val_t (*ternary_t)(val_t x, val_t y, val_t z);
typedef val_t (*nary_t)(val_t *args, arity_t n);

typedef union
{
  thunk_t   thunk;
  unary_t   unary;
  binary_t  binary;
  ternary_t ternary;
  nary_t    nary;
} native_fn_t;

DECL_OBJ(native);
DECL_OBJ_API(native);

struct native_t
{
  OBJ_HEAD;

  native_fn_t native;
};

// forward declarations
obj_t *native(str_t *name, arity_t n, bool vargs, repr_t *repr, native_fn_t native);

#endif
