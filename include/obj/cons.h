#ifndef rascal_cons_h
#define rascal_cons_h

#include "obj.h"
#include "mem.h"

typedef struct str_t str_t;

typedef enum
  {
    cons_fl_proper   =0x0001,
    cons_fl_kv       =0x0002,
    cons_fl_var      =0x0004,
    cons_fl_upval    =0x0008,
    cons_fl_captured =0x0010,
  } cons_fl_t;

typedef struct cons_t
{
  OBJ_HEAD;

  val_t   hd;
  val_t   tl;

  arity_t len;
  type_t  eltype;
} cons_t;

typedef struct kv_t
{
  cons_t  cons;
  hash_t  hash;
} kv_t;

typedef struct var_t
{
  str_t  *doc;
  obj_t  *meta;
} var_t;

// api
cons_t *new_cons( val_t hd, val_t tl );
kv_t   *new_kv( val_t k, val_t b, type_t t, arity_t i, hash_t h );
var_t  *new_var(val_t k, val_t b, type_t t, arity_t i, hash_t h );

// convenience
#define is_cons(val) (is_obj_type(val, cons_type))
#define as_cons(val) ((cons_t*)as_obj(val))

static inline bool is_proper(val_t x)
{
  return is_cons(x)
    && flag_p(as_obj(x)->flags, cons_fl_proper);
}

static inline bool is_kv(val_t x)
{
  return is_cons(x)
    && flag_p(as_obj(x)->flags, cons_fl_kv);
}

static inline bool is_var(val_t x)
{
  return is_cons(x)
    && flag_p(as_obj(x)->flags, cons_fl_var);
}

static inline bool is_upval(val_t x)
{
  return is_cons(x)
    && flag_p(as_obj(x)->flags, cons_fl_upval);
}

static inline bool is_list(val_t x)
{
  return x == NIL || is_cons(x);
}

#endif
