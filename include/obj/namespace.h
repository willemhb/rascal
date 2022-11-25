#ifndef rl_obj_namespace_h
#define rl_obj_namespace_h

#include "obj/object.h"

/* commentary */

/* C types */
struct namespace_t
{
  OBJHEAD;

  namespace_t *next;
  map_t       *locals;
};

/* globals */
extern type_t NamespaceType;

/* API */

/* runtime */
void rl_obj_namespace_init( void );
void rl_obj_namespace_mark( void );

/* convenience */
static inline bool         is_namespace( value_t x ) { return rl_isa(x, &NamespaceType); }
static inline namespace_t *as_namespace( value_t x ) { return (namespace_t*)as_object(x); }

#endif
