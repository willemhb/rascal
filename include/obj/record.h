#ifndef rl_obj_record_h
#define rl_obj_record_h

#include "rascal.h"

#include "vm/object.h"

/* commentary */

/* C types */
struct rl_record_t
{
  RL_OBJ_HEADER;

  vector_t slots;
};

/* globals */

/* API */

/* runtime dispatch */
void rl_obj_record_init( void );
void rl_obj_record_mark( void );
void rl_obj_record_cleanup( void );

/* convenience */

#endif
