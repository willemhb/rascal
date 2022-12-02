#include "vm/error.h"
#include "vm/memory.h"
#include "vm/obj/support/namespc.h"

#include "obj/fixnum.h"
#include "obj/symbol.h"
#include "obj/lambda.h"

/* commentary */

/* instantiations */
bool ns_compare( symbol_t *x, symbol_t *y ) { return x == y; }

#include "tpl/impl/hashmap.h"
HASHMAP(ns_mapping, symbol_t*, int, pad_table_size, get_symbol_hash, ns_compare, NULL, -1);

/* C types */

/* globals */

/* API */
/* constructors */
namespc_t *make_namespc( void )
{
  return alloc(sizeof(namespc_t));
}

void init_namespc( namespc_t *namespc, namespc_t *parent )
{
  namespc->next   = parent;
  namespc->locals = make_ns_mapping(0, NULL);
}

void free_namespc( namespc_t *namespc )
{
  free_ns_mapping(namespc->locals);
  dealloc(namespc, sizeof(namespc_t));
}

int get_namespc_ref( namespc_t *namespc, symbol_t *name, int *offset )
{
  *offset=0; int status;

  while ( namespc )
    {
      if ( (status=ns_mapping_get(namespc->locals, name)) > -1 )
	break;

      (*offset)++;
      namespc = namespc->next;
    }

  return status;
}

int def_namespc_ref( namespc_t *namespc, symbol_t *name )
{
  return ns_mapping_intern(namespc->locals, name, namespc->locals->len);
}

/* runtime dispatch */
void rl_vm_obj_support_namespc_init( void ) {}
void rl_vm_obj_support_namespc_mark( void ) {}
void rl_vm_obj_support_namespc_cleanup( void ) {}

