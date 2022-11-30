#ifndef rl_vm_obj_support_readtable_h
#define rl_vm_obj_support_readtable_h

#include "rascal.h"

/* commentary */

/* C types */
typedef void (*reader_fn_t)(reader_t *reader, int dispatch);

/* API */

/* runtime */
void rl_vm_obj_support_readtable_init( void );
void rl_vm_obj_support_readtable_mark( void );

/* instantiations */
#include "tpl/decl/hashmap.h"

HASHMAP(readtable, int, reader_fn_t);

#endif
