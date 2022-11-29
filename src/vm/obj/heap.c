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
  .preserve_objs=NULL,
  .preserve_vals=NULL,
  .live=NULL,
  .n_alloc=0,
  .n_collect=HEAP_INI
};

/* API */

/* runtime */
void rl_vm_obj_heap_init( void )
{
  Heap.grays = make_objects(0, NULL);
  Heap.preserve_objs = make_objects(0, NULL);
  Heap.preserve_vals = make_values(0, NULL);
}

void rl_vm_obj_heap_mark( void )
{
  for ( size_t i=0; i<Heap.preserve_objs->len; i++ )
    {
      object_t *object  = alist_member(Heap.preserve_objs, i, object_t*);
      mark_object(object);
    }

  for ( size_t i=0; i<Heap.preserve_vals->len; i++ )
    {
      value_t value = alist_member(Heap.preserve_vals, i, value_t);
      mark_value(value);
    }
}

/* convenience */
