#ifndef rascal_obj_h
#define rascal_obj_h

#include "val.h"

struct obj_t
{
  obj_t      *next;
  val_type_t  type  : 30;
  val_type_t  black :  1;
  val_type_t  gray  :  1;
  arity_t     arity;
};

#define OBJ_HEAD \
  obj_t object

// globals --------------------------------------------------------------------
extern obj_t *WellKnownObjects[NUM_TYPES_PAD];

// forward declarations -------------------------------------------------------
void init_obj( obj_t *obj, val_type_t type, flags_t fl );
void mark_obj( obj_t *obj );
void finalize_obj( obj_t *obj );

// toplevel dispatch ----------------------------------------------------------
void obj_mark( void );
void obj_init( void );

// convenience ----------------------------------------------------------------
static inline bool obj_is_static( obj_t *obj )
{
  return obj->next == obj;
}

static inline void *as_ptr( val_t x )
{
  if (val_tag(x) == IMMEDIATE)
    return WellKnownObjects[x&255];

  if (val_tag(x) == OBJECT || val_tag(x) == POINTER)
    return ((void*)(x&~ARITY));

  return NULL;
}

static inline val_t as_val( obj_t *obj )
{
  if (WellKnownObjects[obj->type] == obj)
    return IMMEDIATE|obj->type;

  return ((val_t)obj)|OBJECT;
}

#endif
