#ifndef rl_obj_environment_h
#define rl_obj_environment_h

#include "rascal.h"

#include "vm/object.h"

/* commentary */

/* C types */
struct rl_environment_t
{
  RL_OBJ_HEADER;

  rl_environment_t *next;
  vector_t          bindings;
};

/* globals */

/* API */

/* runtime dispatch */
void rl_obj_environment_init( void );
void rl_obj_environment_mark( void );
void rl_obj_environment_cleanup( void );

/* convenience */

#endif
