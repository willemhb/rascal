#ifndef rl_obj_namespace_h
#define rl_obj_namespace_h

#include "rascal.h"

#include "vm/object.h"

/* commentary */

/* instantiations */
#include "tpl/decl/hashmap.h"
HASHMAP(ns_mapping, rl_symbol_t*, rl_value_t);

/* C types */
struct rl_namespace_t
{
  RL_OBJ_HEADER;

  ns_mapping_t *table;
};

/* globals */

/* API */

/* runtime dispatch */
void rl_obj_namespace_init( void );
void rl_obj_namespace_mark( void );
void rl_obj_namespace_cleanup( void );

/* convenience */

#endif
