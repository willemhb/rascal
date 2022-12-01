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
HASHMAP(ns_mapping, symbol_t*, value_t, pad_table_size, get_symbol_hash, ns_compare, NULL, NUL);

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

int lookup( namespc_t *namespc, symbol_t *key, size_t *i, value_t *bind )
{
  *i    = 0;
  *bind = NUL;

  while ( namespc )
    {
      void **space = ns_mapping_locate(namespc->locals, key);

      if ( hmap_key(space, 0, symbol_t*) != NULL )
	{
	  *bind = hmap_val(space, 0, value_t);
	  return 1;
	}

      (*i)++;
      namespc = namespc->next;
    }

  return -1;
}

int define_variable( namespc_t *namespc, symbol_t *varname )
{
  value_t location = fixnum(namespc->n_vars);
  value_t result   = ns_mapping_intern(namespc->locals, varname, location );

  if ( !is_fixnum(result) ) // name was already bound to a macro
    return -1;

  return as_fixnum(result);
}

int define_macro( namespc_t *namespc, symbol_t *macroname, lambda_t *macro )
{
  ns_mapping_intern(namespc->locals, macroname, tag_object(macro));
  return 0;
}

/* runtime dispatch */
void rl_vm_obj_support_namespc_init( void ) {}
void rl_vm_obj_support_namespc_mark( void ) {}
void rl_vm_obj_support_namespc_cleanup( void ) {}

