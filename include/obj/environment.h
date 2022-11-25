#ifndef rl_obj_environment_h
#define rl_obj_environment_h

#include "obj/object.h"

/* commentary */

/* C types */
struct environment_t
{
  OBJHEAD;

  environment_t *next;
  vector_t      *binds;
};

/* globals */
extern type_t EnvironmentType;

/* API */

/* runtime */
void rl_obj_environment_init( void );
void rl_obj_environment_mark( void );

/* convenience */
static inline bool           is_environment( value_t x ) { return rl_isa(x, &EnvironmentType); }
static inline environment_t *as_environment( value_t x ) { return (environment_t*)as_object(x); }

#endif
