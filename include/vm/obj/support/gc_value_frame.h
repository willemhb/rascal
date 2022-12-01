#ifndef rl_vm_obj_support_gc_value_frame_h
#define rl_vm_obj_support_gc_value_frame_h

#include "rascal.h"

/* commentary */

/* C types */
typedef struct gc_value_frame_t
{
  struct gc_value_frame_t *next;
  size_t n;
  value_t **references;
} gc_value_frame_t;

/* globals */

/* API */
void gc_value_frame_init(gc_value_frame_t *frame, size_t n, value_t **references);
void gc_value_frame_cleanup(gc_value_frame_t *frame);
void mark_gc_value_frames(gc_value_frame_t *frame);

/* runtime */
void rl_vm_obj_support_gc_value_frame_init( void );
void rl_vm_obj_support_gc_value_frame_mark( void );
void rl_vm_obj_support_gc_value_frame_cleanup( void );

/* convenience */


#endif
