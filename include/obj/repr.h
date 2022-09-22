#ifndef rascal_repr_h
#define rascal_repr_h

#include "obj.h"
#include "num.h"

typedef struct repr_t repr_t;

DECL_OBJ(repr);

typedef struct repr_t
{
  OBJ_HEAD;
  // basic parameters ---------------------------------------------------------
  obj_t     *name;
  size_t     base_size;
  val_t      val_tag;
  val_type_t val_type;
  C_type_t   val_C_type;

  // memory management --------------------------------------------------------
  obj_t  *(*new)(init_t *args);
  void    (*init)(obj_t *obj, init_t *args);
  obj_t  *(*resize)(obj_t *obj, size_t n);
  void    (*trace)(obj_t *obj);
  void    (*finalize)(obj_t *obj);

  // other dispatching methods ------------------------------------------------
  int_t   (*cmp)(val_t x, val_t y);
  arity_t (*prin)(obj_t *stream, val_t val);
  hash_t  (*hash)(val_t val);
  val_t   (*call)(val_t val, val_t *args, arity_t n);
} repr_t;

// globals --------------------------------------------------------------------
extern repr_t *Reprs[num_val_types];

#endif
