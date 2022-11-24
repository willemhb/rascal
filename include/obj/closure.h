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


/* initialization */
void rl_closure_init( void );

/* utilities & convenience */
#define is_closure( x )      value_is_type(x, &ClosureType)
#define as_closure( x )     ((closure_t*)as_obj(x))

#endif
