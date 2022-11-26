#ifndef rl_obj_lambda_h
#define rl_obj_lambda_h

#include "obj/object.h"

/* C types */
struct lambda_data_t
{
  vector_t   values;
  bytecode_t code;
};

struct lambda_t
{
  struct object_t      obj;
  struct lambda_data_t data;
};

/* globals */
extern struct type_t LambdaType;

/* API */
lambda_t make_lambda( void );
value_t  lambda_constant( lambda_t lambda, size_t n );

/* runtime */
void rl_obj_lambda_init( void );

/* convenience */
#define is_lambda( x )   (rl_typeof(x)==&LambdaType.data)
#define as_lambda( x )   ((lambda_t)((x)&PTRMASK))

#endif
