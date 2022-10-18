#include "rl/value.h"
#include "obj/object.h"


void *val_as_ptr( value_t x )
{
  switch (x&VT_MASK)
    {
    case PTR_TAG: return (void*)ptr_data(x);
    case OBJ_TAG: return (void*)obj_data(x);
    case HDR_TAG: return (void*)hdr_data(x);
    default:      return NULL;
    }
}

inline object_t *obj_as_ptr( object_t *obj )
{
  return obj;
}

inline void *ptr_as_ptr( void *ptr )
{
  return ptr;
}

void val_mark( value_t val )
{
  if (is_obj(val))
    obj_mark( as_obj(val) );
}
