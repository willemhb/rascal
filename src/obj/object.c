#include <string.h>
#include "obj/object.h"
#include "vm/memory.h"


object_t *obj_new( type_t *type, size_t n, void *data )
{
  object_t *new;
  
  if (type->dtype->new)
      new = type->dtype->new( type, n );

  else
    new = allocob( type_base_size(type) + n );

  init_obj( new, type, n, data );
  return new;
}

void init_obj( object_t *self, type_t *type, size_t n, void *data )
{
  self->dtype  = type;
  self->hdrtag = HDR_BITS;

  if ( type_)
    type->dtype->init(self, type, n, data);

  else if (data)
    memcpy( self->space, data, n );
}

void obj_mark( object_t *obj )
{
  if (obj == NULL)
    return;

  if (obj->black)
    return;

  obj->black = true;

  if (obj->notrace || obtrace(obj) == NULL)
    obj->gray = false;

  else
    push_gray( &Heap, obj );
}

void obj_free( object_t *obj )
{
  
}
