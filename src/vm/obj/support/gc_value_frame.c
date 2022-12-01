
#include "vm/value.h"

#include "vm/obj/heap.h"

#include "vm/obj/support/gc_value_frame.h"

/* commentary */

/* C types */

/* globals */

/* API */
void gc_value_frame_init( gc_value_frame_t *frame, size_t n, value_t **references )
{
  frame->n           = n;
  frame->references  = references;
  frame->next        = Heap.value_frames;
  Heap.value_frames  = frame;
}

void gc_value_frame_cleanup( gc_value_frame_t *frame )
{
  Heap.value_frames = frame->next;
}

void mark_gc_value_frames( gc_value_frame_t *frame )
{
  while (frame)
    {
      for ( size_t i=0; i<frame->n; i++ )
	mark_value(*frame->references[i]);

      frame = frame->next;
    }
}

/* runtime */
void rl_vm_obj_support_gc_value_frame_init( void ) {}
void rl_vm_obj_support_gc_value_frame_mark( void ) {}

