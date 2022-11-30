
#include "vm/value.h"

#include "vm/obj/support/stack.h"

#include "tpl/impl/alist.h"

/* commentary */

/* instantiations */
ALIST(stack, value_t, value_t, pad_stack_size);

/* API */
void mark_stack( stack_t *stack )
{
  for ( size_t i=0; i<stack->len; i++ )
    mark_value(alist_member(stack, i, value_t));
}

/* runtime */
void rl_vm_obj_support_stack_init( void ) {}
void rl_vm_obj_support_stack_mark( void ) {}

