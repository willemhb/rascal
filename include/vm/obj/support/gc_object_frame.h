#ifndef rl_vm_obj_support_gc_object_frame_h
#define rl_vm_obj_support_gc_object_frame_h

#include "rascal.h"

/* commentary

   data structure for preserving objects inside C functions that are live but
   not yet visble to the garbage collector. */

/* C types */
typedef struct gc_object_frame_t
{
  struct gc_object_frame_t *next;
  size_t n;
  object_t ***references;
} gc_object_frame_t;

/* globals */

/* API */
void gc_object_frame_init(gc_object_frame_t *frame, size_t n, object_t ***references);
void gc_object_frame_cleanup(gc_object_frame_t *frame);
void mark_gc_object_frames(gc_object_frame_t *frame);

/* runtime */
void rl_vm_obj_support_gc_object_frame_init( void );
void rl_vm_obj_support_gc_object_frame_mark( void );
void rl_vm_obj_support_gc_object_frame_cleanup( void );

/* convenience */

#endif
