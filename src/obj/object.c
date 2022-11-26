#include <assert.h>
#include <string.h>

#include "obj/object.h"
#include "obj/type.h"

#include "vm/memory.h"

/* commentary */

/* C types */

/* globals */

/* API */
object_t make_object( struct type_t *type )
{
  struct object_t *object = alloc(type->obsize);

  object->type = type;
  object->size = type->obsize;

  /* return pointer to object data */
  return object->space;
}

void free_object( object_t object )
{
  struct object_t *header = obj_head(object);

  dealloc(header, header->size);
}

/* runtime */
void rl_obj_object_init( void ) {}

/* convenience */
