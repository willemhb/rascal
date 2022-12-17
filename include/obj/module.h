#ifndef rl_obj_module_h
#define rl_obj_module_h

#include "rascal.h"

#include "vm/object.h"

/* commentary */

/* instantiations */

/* C types */
struct rl_module_t
{
  RL_OBJ_HEADER;

  rl_string_t *name;
  rl_bytecode_t *bytecode;
  rl_namespace_t *namespace;
  rl_environment_t *environment;

  ulong idno;
};

/* globals */

/* API */

/* runtime dispatch */
void rl_obj_module_init( void );
void rl_obj_module_mark( void );
void rl_obj_module_cleanup( void );

/* convenience */

#endif
