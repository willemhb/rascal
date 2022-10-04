#ifndef rascal_obj_h
#define rascal_obj_h

#include "types.h"
#include "val.h"

// common object flags
typedef enum
  {
    obj_fl_traversed=0x8000,
    obj_fl_frozen   =0x4000,
  } obj_fl_t;

typedef struct obj_t
{
  obj_t    *next;
  flags16_t flags;
  flags8_t  gray;
  flags8_t  black;
  type_t    type;
} obj_t;

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
void trace_objs(obj_t **objs, size_t n);
void free_obj(obj_t *obj);

#endif
