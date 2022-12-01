#include <assert.h>
#include <string.h>

#include "vm/object.h"
#include "obj/type.h"

#include "vm/memory.h"

/* commentary */

/* C types */

/* globals */

/* API */
object_t *make_object( datatype_t *datatype )
{
  layout_t *layout  = datatype->layout;
  
  object_t *out = alloc(layout->obsize);

  *out = (object_t) obj_init(datatype, layout->obsize, 0);

  /* in case allocation of object fiels triggers the GC */
  save_objects(1, &out);

  init_object(out);

  return out;
}

void init_object( object_t *object )
{
  object->next = Heap.live;
  Heap.live    = object;

  if ( object_init(object) )
    object_init(object)(object);
}

void free_object( object_t *object )
{
  if ( object_free(object) )
    object_free(object)(object);

  if ( !flagp(object->flags, object_fl_static))
    dealloc(object, object->size);
}

void mark_object( object_t *object )
{
  if ( object == NULL )
    return;

  if ( object->black )
    return;

  object->black = true;

  mark_object((object_t*)object->type);

  if ( object_trace(object) )
    objects_push( Heap.grays, object );

  else
    object->gray = false;
}

/* accessors */
#include "tpl/impl/record.h"
GET(object, type, datatype_t*);
GET(object, size, size_t);
GET(object, flags, uint);

uint set_object_flags( object_t *object, uint fl )
{
  assert(object);

  return (object->flags |= fl);
}

uint unset_object_flags( object_t *object, uint fl )
{
  assert(object);

  return (object->flags &= ~fl);
}

/* runtime */
void rl_vm_object_init( void )    {}
void rl_vm_object_mark( void )    {}
void rl_vm_object_cleanup( void ) {}

/* convenience */
