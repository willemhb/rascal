#include <assert.h>
#include <string.h>

#include "repr.h"

#include "obj/object.h"
#include "obj/type.h"
#include "obj/stream.h"

#include "vm/memory.h"

#include "util/string.h"

/* commentary */

/* C types */

/* globals */

/* API */
object_t *make_object( type_t *type, size_t n )
{
  if ( type->make_fn )
    return type->make_fn(type, n);

  object_t *out = alloc(type->ob_size);

  *out = (object_t) OBJINIT(type, type->ob_size, true);

  return out;
}

void init_object( object_t *object, size_t n, void *ini )
{
  if ( object->type->init_fn )
    object->type->init_fn(object, n, ini);

  else
    {
      type_t  *type   = object->type;

      assert( n == type->n_fields );

      if ( ini )
	memcpy(object->space, ini, type->ob_size - sizeof(object_t));
    }
}

void free_object( object_t *object )
{
  if ( object == NULL || !object->allocated )
    return;

  object->type->free_fn(object);
}

extern void mark_value( value_t x );
extern void mark_object( object_t *object );

void trace_object( object_t *object )
{
  object->type->trace_fn(object);
  object->gray = false;
}

/* runtime */
void rl_obj_object_init( void ) {}
void rl_obj_object_mark( void ) {}
