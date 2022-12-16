#ifndef rl_obj_closure_h
#define rl_obj_closure_h

#include "rascal.h"


#include "vm/object.h"

/* commentary */

/* C types */
struct rl_closure_t
{
  RL_OBJ_HEADER;

  rl_bytecode_t *bytecode;
  rl_environment_t *environment;
};

/* globals */
extern rl_datatype_t ClosureType;

/* API */
/* constructors */


/* accessors */

/* runtime dispatch */
void rl_obj_closure_init( void );
void rl_obj_closure_mark( void );
void rl_obj_closure_cleanup( void );

/* convenience */
#define is_closure( x ) (rl_typeof(x)==&ClosureType)
#define as_closure( x ) ((rl_closure_t*)as_object(x))

#endif
