#ifndef rl_vm_obj_support_bytecode_h
#define rl_vm_obj_support_bytecode_h

#include "tpl/decl/array.h"

/* commentary

   simple array type for reprsenting Rascal instruction sequences. */

/* instantiations */
ARRAY(bytecode, ushort);

/* C types */

/* globals */

/* API */

/* runtime */
void rl_vm_obj_support_bytecode_init( void );
void rl_vm_obj_support_bytecode_mark( void );

/* convenience */

#endif
