#ifndef rl_vm_obj_support_gc_object_frame_h
#define rl_vm_obj_support_gc_object_frame_h

#include "rascal.h"

/* commentary

   data structure for preserving objects inside C functions that are live but
   not yet visble to the garbage collector. */

/* C types */
typedef struct gc_objs_t
{
  struct gc_objs_t *next;
  size_t n;
  obj_t ***references;
} gc_objs_t;

/* globals */

/* API */
void gc_objs_init(gc_objs_t *frame, size_t n, obj_t ***references);
void gc_objs_cleanup(gc_objs_t *frame);
void mark_gc_objs(gc_objs_t *frame);

/* runtime */
void rl_vm_obj_support_gc_objs_init( void );
void rl_vm_obj_support_gc_objs_mark( void );
void rl_vm_obj_support_gc_objs_cleanup( void );

/* convenience */

#endif
