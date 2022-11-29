#ifndef rl_vm_obj_support_string_h
#define rl_vm_obj_support_string_h

#include "tpl/decl/array.h"

/* commentary

   A simple safe string type for use in the rascal VM. */

/* instantiations */

ARRAY(string, char);

/* C types */

/* globals */

/* API */

/* runtime */
void rl_vm_obj_support_string_init( void );
void rl_vm_obj_support_string_mark( void );

/* convenience */
#define rl_string(name) (struct string_header_t) { sizeof(name)-1, sizeof(name), { name } }

#endif
