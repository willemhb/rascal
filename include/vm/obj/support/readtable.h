#ifndef rl_vm_obj_support_readtable_h
#define rl_vm_obj_support_readtable_h

#include "common.h"
#include "tpl/decl/hashmap.h"

HASHMAP(readtable, char, funcptr);

/* runtime */
void rl_vm_obj_support_readtable_init( void );
void rl_vm_obj_support_readtable_mark( void );

#endif
