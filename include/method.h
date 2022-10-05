#ifndef rascal_method_h
#define rascal_method_h

#include "table.h"

struct metht_t
{
  obj_t obj;
  TABLE_SLOTS(meth_t*);

  arity_t   min_argc;
  arity_t   max_argc;
  func_t   *func;
  meth_t   *tree;
};

typedef enum
  {
    meth_fl_native=0x01,
  } meth_fl_t;

struct meth_t
{
  obj_t obj;
  TABLE_SLOTS(meth_t*);
  ENTRY_SLOTS(types_t *, signature, union { native_fn_t native; }, procedure );
  arity_t argc;
  bool    vargs;
  
  metht_t *metht;
};

struct types_t
{
  obj_t obj;
  ENTRY_SLOTS( type_t *, types, type_t, type );
};



// forward declarations


metht_t *new_metht(void);
void     init_metht(metht_t *metht, func_t *func);
void     mark_metht(obj_t *obj);
void     free_metht(obj_t *obj);
void     resize_metht(meth_t *metht, arity_t newl);
void     rehash_metht(meth_t **new, arity_t );

#endif
