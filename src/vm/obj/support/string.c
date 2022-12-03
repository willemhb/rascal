#include "vm/obj/support/string.h"

#include "util/collection.h"

#include "tpl/impl/array.h"

/* commentary */

/* instantiations */
ARRAY(string, char, uint, pad_string_size);

/* runtime */
void rl_vm_obj_support_string_init( void )    {}
void rl_vm_obj_support_string_mark( void )    {}
void rl_vm_obj_support_string_cleanup( void ) {}
