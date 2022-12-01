#ifndef rl_vm_obj_heap_h
#define rl_vm_obj_heap_h

#include "rascal.h"

#include "vm/obj/support/objects.h"
#include "vm/obj/support/gc_object_frame.h"
#include "vm/obj/support/gc_value_frame.h"

/* commentary

   At this point the heap only keeps track of allocations */

/* C types */
struct heap_t
{
  objects_t         *grays;
  gc_object_frame_t *object_frames;
  gc_value_frame_t  *value_frames;
  object_t          *live;

  bool               collecting;
  
  size_t             n_alloc;
  size_t             n_collect;
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
