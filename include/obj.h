#ifndef rascal_obj_h
#define rascal_obj_h

#include "val.h"
#include "utils/num.h"

#define OBJ_HEAD				\
  obj_t object

struct obj_t
{
  obj_t      *next;
  type_t      type;
  uint16_t    flags;
  uint8_t     black;
  uint8_t     gray;
};

// api
void init_obj(obj_t *obj, type_t type, flags_t fl);

DECL_ALIST(objs, obj_t*);
DECL_ALIST_API(objs, obj_t*);

// convenience ----------------------------------------------------------------
#define as_obj(val) ((obj_t*)as_ptr(val))
#define is_obj(val) (val_tag(val)==OBJECT)

static inline bool  is_obj_type( val_t x, type_t t )
{
  return is_obj(x)
    && as_obj(x)->type == t;
}

#endif
