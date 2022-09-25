#ifndef rascal_obj_h
#define rascal_obj_h

#include "val.h"
#include "utils/num.h"

typedef struct init_t init_t;

#define OBJ_HEAD				\
  obj_t object

#define DECL_OBJ(T)				\
  typedef struct T##_t T##_t

#define DECL_OBJ_API(T)							\
  void   mark_##T(obj_t *o);						\
  void   finalize_##T(obj_t *o);					\
  obj_t *resize_##T(obj_t *o, size_t o_n, size_t n_n);			\
  void   T##_init( void );						\
  void	 T##_mark( void )

#define IMPL_MARK(T)     void   mark_##T(obj_t *obj)
#define IMPL_FINALIZE(T) void   finalize_##T(obj_t *obj)
#define IMPL_RESIZE(T)   void   resize_##T(obj_t *obj, arity_t o_n, arity_t n_n)
#define INIT_IMPL(T)     void   T##_init( void )
#define INIT_MARK(T)     void   T##_mark( void )

DECL_OBJ(obj);
DECL_OBJ_API(obj);

struct obj_t
{
  obj_t      *next;
  type_t      type;
  uint16_t    flags;
  uint8_t     black;
  uint8_t     gray;
};

size_t sizeof_obj(obj_t *obj);
bool   is_static(obj_t *obj);
bool   has_static(obj_t *obj);
bool   is_famous(obj_t *obj);

void   init_obj( obj_t *obj, type_t type, flags_t fl );

// globals --------------------------------------------------------------------
extern obj_t *FamousObjects[NUM_TYPES_PAD];

// convenience ----------------------------------------------------------------
#define as_obj(val) ((obj_t*)as_ptr(val))
#define is_obj(val) (val_tag(val)==OBJECT)

static inline void *as_ptr( val_t x )
{
  if (val_tag(x) == IMMEDIATE)
    return FamousObjects[x&255];

  if (val_tag(x) == OBJECT || val_tag(x) == POINTER)
    return ((void*)(x&~ARITY));

  return NULL;
}

static inline bool  is_obj_type( val_t x, type_t t )
{
  return is_obj(x)
    && as_obj(x)->type == t;
}


static inline val_t as_val( obj_t *obj )
{
  if (is_famous(obj))
    return IMMEDIATE|obj->type;

  return ((val_t)obj)|OBJECT;
}

#endif
