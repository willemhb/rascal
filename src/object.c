#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>

#include "object.h"
#include "memory.h"
#include "table.h"


/* APIS */
object_t *make_obj( type_t *type )
{
  size_t    base  = type->ob_size;
  object_t *new   = alloc(base);

  /* initialize type, metadata, GC flags */
  new->type  = type;
  new->_meta = NUL;
  new->gray  = true;
  new->black = false;
  new->size  = base;
  new->flags = 0;

  /* return new object */
  return new;
}

void init_obj( object_t *obj, size_t n, void *ini )
{
  if ( obj->type->layout == array_layout )
    {
      
    }
}

void free_obj( object_t *obj )
{
  type_t      *type  = obj->type;
  fieldspec_t *specs = type->fields;
  void        *ptr;

  for (size_t i=0; i<type->n_fields; i++)
    {
      fieldspec_t spec = specs[i];

      switch ( spec.repr )
	{
	case repr_Cdata: case repr_value:
	  break;

	case repr_Cptr:   case repr_Cstring:
	case repr_values: case repr_objects:
	  ptr = *(void**)(obj->space + spec.offset);
	  free(ptr);
	  break;
	}
    }

  // TODO: this is misleading, since obj->size includes regions that were freed @44-46
  dealloc(obj, obj->size);
}

