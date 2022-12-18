#ifndef rl_vm_obj_support_vals_h
#define rl_vm_obj_support_vals_h

#include "tpl/decl/alist.h"

/* commentary

   Arraylist type for tagged values.

   Differs from stack in that it uses a more conservative overallocation scheme. */

/* instantiations */
ALIST(vals, val_t);

/* C types */

/* globals */

/* API */
void mark_vals( vals_t *values );

/* runtime */
void rl_vm_obj_support_vals_init( void );
void rl_vm_obj_support_vals_mark( void );
void rl_vm_obj_support_vals_cleanup( void );

/* convenience */

#endif
