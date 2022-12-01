#ifndef rl_vm_obj_support_readtable_h
#define rl_vm_obj_support_readtable_h

#include "rascal.h"

/* commentary */

/* C types */

/* API */

/* runtime */
void rl_vm_obj_support_readtable_init( void );
void rl_vm_obj_support_readtable_mark( void );
void rl_vm_obj_support_readtable_cleanup( void );

/* instantiations */
#include "tpl/decl/hashmap.h"

HASHMAP(readtable, int, reader_dispatch_fn_t);

#endif
