#ifndef rl_obj_closure_h
#define rl_obj_closure_h

#include "rascal.h"


#include "vm/object.h"

/* commentary */

/* C types */
struct closure_t
{
  object_t  obj;

  lambda_t *function;
  envt_t   *envt;
};

/* globals */
extern datatype_t ClosureType;

/* API */
/* constructors */
value_t capture_closure( lambda_t *lambda, envt_t *envt );

/* accessors */
lambda_t *get_closure_function( closure_t *closure );
envt_t   *get_closure_envt( closure_t *closure );

int       check_closure_argco( closure_t *closure, int argc );

/* runtime dispatch */
void rl_obj_closure_init( void );
void rl_obj_closure_mark( void );
void rl_obj_closure_cleanup( void );

/* convenience */
#define is_closure( x ) (rl_typeof(x)==&ClosureType)
#define as_closure( x ) ((closure_t*)as_object(x))

#endif
