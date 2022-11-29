#include "vm/obj/support/string.h"

#include "obj/type.h"

#include "util/string.h"

/* commentary */

/* C types */

/* globals */
void init_type( object_t *object );
void free_type( object_t *object );

vtable_t TypeMethods =
  {
    .init=init_type,
    .free=free_type
  };

layout_t TypeLayout =
  {
    vmtype_objptr,
    0,
    sizeof(datatype_t)
  };

datatype_t TypeType =
  {
    {
      obj_init( &TypeType, sizeof(datatype_t), object_fl_static ),
      "type"
    },
    &TypeLayout,
    &TypeMethods
  };

/* API */
void init_type( object_t *object )
{
  datatype_t *datatype = (datatype_t*)object;

  if ( flagp(object->flags, object_fl_static) )
      datatype->type.name = make_string(strlen8(datatype->type.name),
					datatype->type.name);
  
  else
      datatype->type.name = make_string(strlen8("<type>"), "<type>");
}

void free_type( object_t *object )
{
  free_string(type_name(object));
}

/* runtime */
void rl_obj_type_init( void )
{
  gl_init_type(TypeType);
}

void rl_obj_type_mark( void )
{
  gl_mark_type(TypeType);
}

/* convenience */
