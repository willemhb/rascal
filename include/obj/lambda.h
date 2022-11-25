#ifndef rl_obj_lambda_h
#define rl_obj_lambda_h

#include "obj/object.h"

/* C types */
struct lambda_t
{
  OBJHEAD;

  namespace_t *namespace;   // lexical environment
  vector_t    *constants;   // constant store
  bytecode_t  *bytecode;    // instruction sequence
};

/* globals */
extern type_t LambdaType;

/* API */

/* runtime */
void rl_obj_lambda_init( void );
void rl_obj_lambda_mark( void );

/* convenience */
static inline bool      is_lambda( value_t x ) { return rl_isa(x, &LambdaType); }
static inline lambda_t *as_lambda( value_t x ) { return (lambda_t*)as_object(x); }

#endif
