#include "vm/obj/support/bytecode.h"

#include "tpl/impl/array.h"

/* commentary

   simple array type for reprsenting Rascal instruction sequences. */

/* instantiations */
ARRAY(bytecode, ushort, uint, pad_alist_size);

/* C types */

/* globals */

/* API */

/* runtime */
void rl_vm_obj_support_bytecode_init( void )    {}
void rl_vm_obj_support_bytecode_mark( void )    {}
void rl_vm_obj_support_bytecode_cleanup( void ) {}

/* convenience */
