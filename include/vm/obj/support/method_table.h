#ifndef rl_vm_obj_support_method_table_h
#define rl_vm_obj_support_method_table_h

#include "rascal.h"

#include "vm/obj/support/signature.h"
#include "vm/obj/support/method.h"

/* commentary */

/* instantiations */
#include "tpl/decl/hashmap.h"
HASHMAP(method_cache, signature_t, method_t);

/* C types */
typedef struct method_table_t method_table_t;

struct method_table_t
{
  method_cache_t *cache;
  method_t       *farg_methods;
  method_t       *varg_methods;
  method_t       *thunk_method;
};

/* globals */

/* API */

/* runtime dispatch */
void rl_vm_obj_support_method_table_init( void );
void rl_vm_obj_support_method_table_mark( void );
void rl_vm_obj_support_method_table_cleanup( void );

/* convenience */

#endif
