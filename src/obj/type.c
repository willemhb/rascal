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
/* object runtime methods */
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
  free_string(get_datatype_name(object));
}

/* accessors */
#include "tpl/impl/record.h"
GET(type, name, string_t);

object_runtime_fn_t get_datatype_init( datatype_t *datatype )
{
  return datatype->methods->init;
}

object_runtime_fn_t get_datatype_trace( datatype_t *datatype )
{
  return datatype->methods->trace;
}

object_runtime_fn_t get_datatype_free( datatype_t *datatype )
{
  return datatype->methods->free;
}

size_t get_datatype_obsize( datatype_t *datatype )
{
  return datatype->layout->obsize;
}

uint get_datatype_flags( datatype_t *datatype )
{
  return datatype->layout->flags;
}

vmtype_t get_datatype_vmtype( datatype_t *datatype )
{
  return datatype->layout->vmtype;
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
