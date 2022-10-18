#include "obj/object.h"


object_t *new_obj( type_t *type, size_t n )
{
  assert( is_obj_type(type) );

  object_t *out;
  
  if (type->dtype->new)
      out = type->dtype->new( type, n );

  
}
