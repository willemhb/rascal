#include "vm/object.h"
#include "vm/value.h"

#include "vm/obj/heap.h"


/* commentary */

/* C types */

/* globals */
#define HEAP_INI UINT16_MAX

struct heap_t Heap =
{
  .grays=NULL,
  .object_frames=NULL,
  .value_frames=NULL,
  .live=NULL,
  .n_alloc=0,
  .n_collect=HEAP_INI
};

/* API */

/* runtime */
void rl_vm_obj_heap_init( void )
{
  Heap.grays = make_objects(0, NULL);
}

void rl_vm_obj_heap_mark( void )
{
  mark_gc_object_frames(Heap.object_frames);
  mark_gc_value_frames(Heap.value_frames);
}

/* convenience */
