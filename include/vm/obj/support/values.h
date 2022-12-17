#ifndef rl_vm_obj_support_values_h
#define rl_vm_obj_support_values_h

#include "tpl/decl/alist.h"

/* commentary

   Arraylist type for tagged values.

   Differs from stack in that it uses a more conservative overallocation scheme. */

/* instantiations */
ALIST(values, rl_value_t);

/* C types */

/* globals */

/* API */
void mark_values( values_t *values );

/* runtime */
void rl_vm_obj_support_values_init( void );
void rl_vm_obj_support_values_mark( void );
void rl_vm_obj_support_values_cleanup( void );

/* convenience */

#endif
