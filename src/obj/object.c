#include "obj/object.h"
#include "obj/type.h"
#include "obj/array.h"
#include "obj/table.h"
#include "obj/hamt.h"
#include "obj/cvalue.h"

repr_t obj_repr( object_t *obj )
{
  assert( obj != NULL );

  return obj->objtag;
}

type_t *obj_type( object_t *obj )
{
  assert( obj != NULL );

  if (obj->objtag < SYMBOL)
    return ((boxed_t*)obj)->type;

  return ReprTypes[obj->objtag];
}

size_t obj_size( object_t *obj )
{
  assert( obj != NULL );

  type_t *type       = obj_type( obj );
  dtype_impl_t *impl = type->impl.dtype;

  switch (impl->repr)
    {
    case ARRAY:
      return impl->impl.arr->asize( (array_t*)obj );

    case TABLE:
      return impl->impl.tab->tbsize( (table_t*)obj );

    case HAMT:
      return impl->impl.hamt->htsize( (hamt_t*)obj );

    case AMT:
      return impl->impl.amt->atsize( (amt_t*)obj );

    case CVALUE:
      if (impl->impl.cval->cvsize)
	return impl->impl.cval->cvsize( (cvalue_t*)obj );

      return impl->base_size;

    default:
      if (impl->impl.obj->obsize)
	return impl->impl.obj->obsize( obj );

      return impl->base_size;
    }
}

size_t obj_unbox( object_t *obj, size_t size, void *spc )
{
  assert( obj != NULL );
  
  if (obj->objtag == CVALUE)
    return cv_unbox( (cvalue_t*)obj, size, spc );

  if (size < 8)
    return 0;

  *(object_t**)spc = obj;
  return 8;
}

rl_value_t obj_unwrap( object_t *obj )
{
  assert( obj != NULL );

  if (obj->objtag == CVALUE)
    return cv_unwrap( (cvalue_t*) obj );

  return (rl_value_t)obj;
}

void obj_mark( object_t *obj )
{
  if (obj == NULL)
    return;

  
}

void obj_free( object_t *obj )
{
  
}
