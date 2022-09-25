#ifndef rascal_cons_h
#define rascal_cons_h

#include "obj.h"
#include "mem.h"

DECL_OBJ(cons);
DECL_OBJ_API(cons);
DECL_VAL_API(cons);


typedef struct pair_t
{
  OBJ_HEAD;

  val_t  fst;
  val_t  snd;

  hash_t hash;
} pair_t;

struct cons_t
{
  OBJ_HEAD;

  val_t   hd;
  cons_t *tl;

  arity_t len;
  type_t  eltype;
};

// convenience
#define is_cons(val) (is_obj_type(val, cons_type))

static inline bool is_list(val_t x)
{
  return x == NIL || is_cons(x);
}

#endif
