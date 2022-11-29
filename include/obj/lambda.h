#ifndef rl_obj_lambda_h
#define rl_obj_lambda_h

#include "vm/object.h"
#include "vm/obj/support/bytecode.h"
#include "vm/obj/support/vector.h"

/* commentary

   compiled code representation. */

/* C types */
struct lambda_t
{
  object_t obj;

  vector_t   constants;
  bytecode_t instructions;
};

/* globals */
extern datatype_t LambdaType;

/* API */
lambda_t make_lambda( void );
value_t  lambda_constant( lambda_t lambda, size_t n );

/* runtime */
void rl_obj_lambda_init( void );
void rl_obj_lambda_mark( void );

/* convenience */
#define is_lambda( x )   (rl_typeof(x)==&LambdaType.data)
#define as_lambda( x )   ((lambda_t)((x)&PTRMASK))

#endif
