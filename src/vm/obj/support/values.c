#include "vm/obj/support/values.h"

#include "tpl/impl/alist.h"

/* commentary */

/* instantiations */
ALIST(values, value_t, value_t, pad_alist_size);

/* runtime */
void rl_vm_obj_support_values_init( void ) {}
void rl_vm_obj_support_values_mark( void ) {}
