#ifndef rascal_cons_h
#define rascal_cons_h

#include "obj.h"

DECL_OBJ(cons);

struct cons_t
{
  OBJ_HEAD;
  union
  {
    val_t car;
    val_t hd;
  };

  union
  {
    val_t   cdr;
    cons_t *tl;
  };

  arity_t    len;
  val_type_t eltype;
};

// convenience
#define is_cons(val) (is_obj_type(val, cons_type))

static inline bool is_list(val_t x)
{
  return x == NIL || is_cons(x);
}

#endif
