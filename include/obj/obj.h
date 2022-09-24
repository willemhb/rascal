#ifndef rascal_obj_h
#define rascal_obj_h

#include "val.h"
#include "num.h"

typedef struct init_t init_t;

#define OBJ_HEAD \
  obj_t object

#define DECL_OBJ(T)						\
  typedef struct T##_t T##_t;					\
  obj_t *new_##T(init_t *args);					\
  void   init_##T(obj_t *obj, init_t *args);			\
  void   write_##T(void *spc, val_t val, Ctype_t Ctype);	\
  void   mark_##T(obj_t *obj);					\
  void   finalize_##T(obj_t *obj);				\
  obj_t *resize_##T(obj_t *obj, size_t old_n, size_t new_n);	\
  void   T##_init( void );					\
  void	 T##_mark( void )

#define IMPL_NEW(T)      obj_t *new_##T(init_t *args)
#define IMPL_INIT(T)     void   init_##T(obj_t *obj, init_t *args)
#define IMPL_WRITE(T)    void   write_##T();
#define IMPL_MARK(T)     void   mark_##T(obj_t *obj)
#define IMPL_FINALIZE(T) void   finalize_##T(obj_t *obj)
#define IMPL_RESIZE(T)   void   resize_##T(obj_t *obj)
#define INIT_IMPL(T)     void   T##_init( void )
#define INIT_MARK(T)     void   T##_mark( void )

DECL_OBJ(obj);

struct obj_t
{
  obj_t      *next;
  val_type_t  type;
  uint16_t    flags;
  uint8_t     black;
  uint8_t     gray;
};

size_t sizeof_obj(obj_t *obj);
bool   is_static(obj_t *obj);
bool   has_static(obj_t *obj);
bool   is_famous(obj_t *obj);

// globals --------------------------------------------------------------------
extern obj_t *WellKnownObjects[NUM_TYPES_PAD];

// convenience ----------------------------------------------------------------
#define as_obj(val) ((obj_t*)as_ptr(x))
#define is_obj(val) (val_tag(val)==OBJECT)

static inline void *as_ptr( val_t x )
{
  if (val_tag(x) == IMMEDIATE)
    return WellKnownObjects[x&255];

  if (val_tag(x) == OBJECT || val_tag(x) == POINTER)
    return ((void*)(x&~ARITY));

  return NULL;
}

static inline bool  is_obj_type( val_t x, val_type_t t )
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
