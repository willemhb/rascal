#ifndef rl_vm_obj_support_namespc_h
#define rl_vm_obj_support_namespc_h

#include "rascal.h"

/* commentary

   The compile-time representation of environments.

   namespc keys are symbols. Namespc bindings are either
   numbers (location in runtime environment) or closures
   (macros).

   For the time being namespc representations are uniform,
   ie, there's nothing special about the toplevel namespc except that
   it's the last one (ns->next is NULL).

   */

/* instantiations */
#include "tpl/decl/hashmap.h"
HASHMAP(ns_mapping, symbol_t*, int);

/* C types */
typedef struct namespc_t namespc_t;

struct namespc_t
{
  namespc_t  *next;
  ns_mapping_t *locals;
};

/* globals */

/* API */
/* memory API */
namespc_t *make_namespc( void );
void       init_namespc( namespc_t *namespc, namespc_t *parent );
void       free_namespc( namespc_t *namespc );

/* accessors and lookup */
int        get_namespc_ref( namespc_t *namespc, symbol_t *name, int *offset );
int        def_namespc_ref( namespc_t *namespc, symbol_t *name );

/* runtime dispatch */
void rl_vm_obj_support_namespc_init( void );
void rl_vm_obj_support_namespc_mark( void );
void rl_vm_obj_support_namespc_cleanup( void );

/* convenience */

#endif
