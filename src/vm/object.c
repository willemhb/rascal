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

  init_object(out);

  return out;
}

void init_object( object_t *object )
{
  object->next = Heap.live;
  Heap.live    = object;

  /* to protect against a potential premature free if initialization triggers the GC */
  object->black = true;
  object->gray  = false;

  if ( object_init(object) )
    object_init(object)(object);

  object->black = false;
  object->gray  = true;
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

/* runtime */
void rl_vm_object_init( void ) {}
void rl_vm_object_mark( void ) {}

/* convenience */
