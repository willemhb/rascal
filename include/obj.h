#ifndef rascal_obj_h
#define rascal_obj_h

#include "types.h"
#include "val.h"

// common object flags
typedef struct obj_t
{
  obj_t    *next;
  flags16_t flags;
  uint8_t   black;
  uint8_t   gray;
  type_t    type;
} obj_t;

// common flags (these occupy the low bits of obj->next & ) 
#define is_obj(val)      (((val)&TMASK)==OBJ)
#define as_obj(val)      ((obj_t*)as_ptr(val))

#define obj_type(val)    (as_obj(val)->type)
#define obj_next(val)    (as_obj(val)->next)
#define obj_gray(val)    (as_obj(val)->gray)
#define obj_black(val)   (as_obj(val)->black)
#define obj_flags(val)   (as_obj(val)->flags)

// describe macros
#define OBJ_NEW(T)				\
  T##_t *new_##T(void)				\
  {						\
    return alloc(sizeof(T##_t));		\
  }

// forward declarations
void init_obj(obj_t *obj, type_t type, flags_t flags);
void mark_obj(obj_t *obj);
void mark_objs(obj_t **objs, size_t n);
void free_obj(obj_t *obj);

#endif
