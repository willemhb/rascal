#include "vm/value.h"

#include "vm/obj/support/values.h"

#include "tpl/impl/alist.h"

/* commentary */

/* instantiations */
ALIST(values, value_t, value_t, pad_alist_size);

/* API */
void mark_values( values_t *values )
{
  for ( size_t i=0; i<values->len; i++ )
      mark_value(alist_member(values, i, value_t));
}

/* runtime */
void rl_vm_obj_support_values_init( void ) {}
void rl_vm_obj_support_values_mark( void ) {}
