#ifndef rascal_obj_closure_h
#define rascal_obj_closure_h

#include "obj/object.h"

/* C types */
struct closure_t
{
  OBJHEAD;
  lambda_t      *function;
  environment_t *environment;
};

/* globals */
extern type_t ClosureType;

/* API */

/* runtime */
void rl_obj_closure_init( void );
void rl_obj_closure_mark( void );

/* convenience */
static inline bool       is_closure( value_t x ) { return rl_isa(x, &ClosureType); }
static inline closure_t *as_closure( value_t x ) { return (closure_t*)as_object(x); }

#endif
