
#include "vm/object.h"

#include "vm/obj/heap.h"

#include "vm/obj/support/gc_object_frame.h"

/* commentary */

/* C types */

/* globals */

/* API */
void gc_object_frame_init( gc_object_frame_t *frame, size_t n, object_t ***references )
{
  frame->n           = n;
  frame->references  = references;
  frame->next        = Heap.object_frames;
  Heap.object_frames = frame;
}

void gc_object_frame_cleanup( gc_object_frame_t *frame )
{
  Heap.object_frames = frame->next;
}

void mark_gc_object_frames( gc_object_frame_t *frame )
{
  while (frame)
    {
      for ( size_t i=0; i<frame->n; i++ )
	mark_object(*frame->references[i]);

      frame = frame->next;
    }
}

/* runtime */
void rl_vm_obj_support_gc_object_frame_init( void ) {}
void rl_vm_obj_support_gc_object_frame_mark( void ) {}

