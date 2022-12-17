#ifndef rl_obj_module_h
#define rl_obj_module_h

#include "rascal.h"

#include "vm/object.h"
#include "vm/obj/support/bytecode.h"
#include "vm/obj/support/values.h"

/* commentary */
/* C types */
struct rl_module_t
{
  RL_OBJ_HEADER;

  rl_symbol_t *name;
  rl_namespace_t *macros; /* runtime */
  rl_namespace_t *variables;
  values_t *bindings;
  bytecode_t *bytecode;
};

/* globals */

/* API */
/* constructors */

/* accessors */

/* runtime dispatch */
void rl_obj_module_init( void );
void rl_obj_module_mark( void );
void rl_obj_module_cleanup( void );

/* convenience */
#define is_module( x )  has_object_type(x, module_object)
#define as_module( x ) ((rl_module_t*)as_object(x))

#endif
