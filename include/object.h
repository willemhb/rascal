#ifndef rascal_obj_h
#define rascal_obj_h

#include "types.h"
#include "value.h"

// common object flags
#define BLACK_FL 0x0000000000000004ul
#define GRAY_FL  0x0000000000000008ul

struct object_t
{
  union
  {
    value_t next;
    struct
    {
      value_t flags :  2;
      value_t black :  1;
      value_t gray  :  1;
      value_t pad   : 64;
    };
  };
};

// convenience
static inline bool is_obj(value_t value)
{
  switch (value&TMASK)
    {
    case TAG_ATOM: return !(value&7);
    case TAG_PNTR: return false;
    case TAG_LIST: return value != NUL;
    default:       return true;
    }
}

#define as_obj(val)      ((object_t*)as_ptr(val))

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
void init_obj(object_t *obj, flags_t flags);
void mark_obj(object_t *obj);
void mark_objs(object_t **objs, size_t n);
void free_obj(object_t *obj);

#endif
