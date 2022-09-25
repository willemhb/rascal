#ifndef rascal_repr_h
#define rascal_repr_h

#include "obj.h"
#include "obj/str.h"
#include "mem.h"
#include "utils/num.h"

typedef struct repr_t repr_t;

DECL_OBJ(repr);
DECL_OBJ_API(repr);

struct repr_t
{
  OBJ_HEAD;
  // basic parameters ---------------------------------------------------------
  obj_t  *name;
  size_t  base_size;
  val_t   val_tag;

  type_t  val_type;
  type_t  el_type;

  Ctype_t val_Ctype;
  Ctype_t el_Ctype;

  // memory management methods ------------------------------------------------
  obj_t  *(*do_resize)(obj_t *obj, size_t n);
  void    (*do_trace)(obj_t *obj);
  void    (*do_finalize)(obj_t *obj);

  // other dispatching methods ------------------------------------------------
  int_t   (*do_cmp)(val_t x, val_t y);
  arity_t (*do_prin)(obj_t *stream, val_t val);
  hash_t  (*do_hash)(val_t val);
  val_t   (*do_call)(val_t val, val_t *args, arity_t n);
};

// globals --------------------------------------------------------------------
extern repr_t *Reprs[num_val_types];

// convenience
char *repr_name( repr_t *repr );
char *type_name( type_t  type );

#define typename(t)				\
  _Generic((t),					\
	   repr_t*:repr_name,			\
	   type_t:type_name)(t)

#endif
