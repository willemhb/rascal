#include "vm/obj/support/vector.h"

#include "tpl/impl/array.h"

/* commentary */

/* instantiations */

ARRAY(vector, value_t, value_t, pad_alist_size);

/* API */
void mark_vector(vector_t vector)
{
  for ( size_t i=0; i <vector_len(vector); i++ )
    mark_value(vector[i]);
}

/* runtime */
void rl_vm_obj_support_vector_init( void ) {}
void rl_vm_obj_support_vector_mark( void ) {}

