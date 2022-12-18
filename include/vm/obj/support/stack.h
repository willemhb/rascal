#ifndef rl_vm_obj_support_stack_h
#define rl_vm_obj_support_stack_h

#include "tpl/decl/alist.h"

/* commentary */

/* instantiations */
ALIST(stack, val_t);

/* C types */

/* globals */

/* API */
void mark_stack( stack_t *stack );

/* runtime */
void rl_vm_obj_support_stack_init( void );
void rl_vm_obj_support_stack_mark( void );
void rl_vm_obj_support_stack_cleanup( void );

/* convenience */

#endif
