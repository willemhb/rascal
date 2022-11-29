#include "vm/obj/support/stack.h"

#include "tpl/impl/alist.h"

/* commentary */

/* instantiations */
ALIST(stack, value_t, value_t, pad_stack_size);

/* runtime */
void rl_vm_obj_support_stack_init( void ) {}
void rl_vm_obj_support_stack_mark( void ) {}

