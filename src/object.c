#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "object.h"
#include "memory.h"
#include "table.h"


/* APIS */
object_t *make_obj( type_t *type )
{
  size_t    total = type->size;
  object_t *new   = alloc(total);

  new->type  = type;
  new->size  = total;
  new->_meta = NUL;
  new->black = false;
  new->gray  = true;
  new->flags = 0;

  return new;
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

