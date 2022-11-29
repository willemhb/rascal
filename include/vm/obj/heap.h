#ifndef rl_vm_obj_heap_h
#define rl_vm_obj_heap_h

#include "rascal.h"

#include "vm/obj/support/objects.h"
#include "vm/obj/support/values.h"

/* commentary

   At this point the heap only keeps track of allocations */

/* C types */
struct heap_t
{
  objects_t *grays;
  objects_t *preserve_objs;
  values_t  *preserve_vals;

  object_t  *live;

  bool       collecting;
  
  size_t     n_alloc;
  size_t     n_collect;
};

/* globals */
extern heap_t Heap;

/* API */

/* runtime */
void rl_vm_obj_heap_init( void );
void rl_vm_obj_heap_mark( void );

/* convenience */
#define save_objs( n, ... ) objects_pushn(Heap.preserve_objs, (n) __VA_OPT__(,) __VA_ARGS__)
#define save_vals( n, ... ) values_pushn(Heap.preserve_vals, (n) __VA_OPT__(,) __VA_ARGS__)
#define unsave_objs( n )    objects_popn(Heap.preserve_objs, (n))
#define unsave_vals( n )    values_popn(Heap.preserve_vals, (n))

#endif
