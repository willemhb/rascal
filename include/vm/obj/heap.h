#ifndef rl_vm_obj_heap_h
#define rl_vm_obj_heap_h

#include "rascal.h"

#include "vm/obj/support/objs.h"
#include "vm/obj/support/gc_objs.h"
#include "vm/obj/support/gc_vals.h"

/* commentary

   At this point the heap only keeps track of allocations */

/* C types */
struct heap_t
{
  objs_t     *grays;
  gc_objs_t *save_objs;
  gc_vals_t *save_vals;
  obj_t     *live;

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
void rl_vm_obj_heap_cleanup( void );

/* convenience */

#endif
