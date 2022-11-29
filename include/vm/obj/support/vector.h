#ifndef rl_vm_obj_support_vector_h
#define rl_vm_obj_support_vector_h

#include "vm/value.h"

#include "tpl/decl/array.h"

/* commentary

   Underlying array type for arrays of tagged values. */

/* instantiations */
ARRAY(vector, value_t);

/* C types */

/* globals */
void mark_vector( vector_t vector );

/* runtime */
void rl_vm_obj_support_vector_init( void );
void rl_vm_obj_support_vector_mark( void );

#endif
