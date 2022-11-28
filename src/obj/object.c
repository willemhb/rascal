#include <assert.h>
#include <string.h>

#include "obj/object.h"
#include "obj/type.h"

#include "vm/memory.h"

/* commentary */

/* C types */

/* globals */

/* API */
object_t make_object( type_t type, size_t extra )
{
  size_t base_size = type_base_size(type);
  object_t out     = (uchar*)alloc(base_size + extra) + type->hdsize;

  obj_type(out) = type;
  obj_size(out) = base_size + extra;

  return out;
}

void free_object( object_t object )
{
  uchar *space = object - obj_type(object)->hdsize;
  size_t alloc = obj_size(object);

  dealloc(space, alloc);
}

/* runtime */
void rl_obj_object_init( void ) {}

/* convenience */
void *obj_start( object_t object )
{
  /* get start of hidden fields. */
  
  return object - obj_type(object)->hdsize;
}
