#include <assert.h>
#include <string.h>

#include "repr.h"

#include "obj/object.h"
#include "obj/type.h"
#include "obj/stream.h"

#include "vm/vm.h"
#include "vm/heap.h"
#include "vm/memory.h"

#include "util/string.h"

/* commentary */

/* C types */

/* globals */

/* API */
object_t *make_object( type_t *type, size_t n, uint flags, void *ini )
{
  return type->make_fn(type, n, flags, ini);
}

void init_object( object_t *object, type_t *type, size_t n, uint flags, void *ini )
{
  flags   |= type->is_leaf * mem_is_leaf;

  *object  = (object_t) { Heap.live_objects, NUL, type, flags&(3), type->ob_size, true, false };

  Heap.live_objects = object;

  type->init_fn(object, type, n, flags, ini);
}

void free_object( object_t *object )
{
  if ( object == NULL || !flagp(object->flags, mem_is_alloc) )
    return;

  object->type->free_fn(object);

  dealloc(object, object->type->ob_size);
}

void trace_object( object_t *object )
{
  object->type->trace_fn(object);
  object->gray = false;
}

/* runtime */
void rl_obj_object_init( void ) {}
void rl_obj_object_mark( void ) {}
