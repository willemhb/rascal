#ifndef rl_vm_obj_support_gc_vals_h
#define rl_vm_obj_support_gc_vals_h

#include "rascal.h"

/* commentary */

/* C types */
typedef struct gc_vals_t
{
  struct gc_vals_t *next;
  size_t n;
  val_t **references;
} gc_vals_t;

/* globals */

/* API */
void gc_vals_init(gc_vals_t *frame, size_t n, val_t **references);
void gc_vals_cleanup(gc_vals_t *frame);
void mark_gc_valss(gc_vals_t *frame);

/* runtime */
void rl_vm_obj_support_gc_vals_init( void );
void rl_vm_obj_support_gc_vals_mark( void );
void rl_vm_obj_support_gc_vals_cleanup( void );

/* convenience */


#endif
