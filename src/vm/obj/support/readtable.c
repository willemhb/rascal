#include "vm/obj/support/readtable.h"

#include "tpl/impl/hashmap.h"

/* commentary */

/* instantiations */
bool equal_chars( int x, int y );

HASHMAP(readtable, char, funcptr, pad_table_size, hash_int32 , equal_chars, EOF, NULL);

/* C types */

/* globals */

/* API */
bool equal_chars( int x, int y )
{
  return x == y;
}

/* runtime */
void rl_vm_obj_support_readtable_init( void ) {}
void rl_vm_obj_support_readtable_mark( void ) {}

/* convenience */
