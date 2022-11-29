#include <stdarg.h>

#include "obj/lambda.h"
#include "obj/type.h"

/* commentary */

/* C types */

/* globals */
#define N_CONSTANTS 16
#define N_INSTRUCTIONS 64

void init_lambda( object_t *object );
void free_lambda( object_t *object );
void trace_lambda( object_t *object );

struct vtable_t LambdaMethods =
  {
   .init=init_lambda,
   .free=free_lambda,
   .trace=trace_lambda
  };

struct layout_t LambdaLayout =
  {
   .vmtype=vmtype_objptr,
   .flags=0,
   .obsize=sizeof(lambda_t)
  };

struct datatype_t LambdaType =
  {
   {
    .obj=obj_init(&TypeType, sizeof(datatype_t), object_fl_static),
    .name="lambda"
   },

   .layout=&LambdaLayout,
   .methods=&LambdaMethods
  };

/* API */
void init_lambda( object_t *object )
{
  lambda_t *lambda = (lambda_t*)object;

  lambda->constants = make_vector(N_CONSTANTS, NULL);
  lambda->instructions = make_bytecode(N_INSTRUCTIONS, NULL);
}

void free_lambda( object_t *object )
{
  lambda_t *lambda = (lambda_t*)object;

  free_vector(lambda->constants);
  free_bytecode(lambda->instructions);
}

void trace_lambda( object_t *object )
{
  lambda_t *lambda = (lambda_t*)object;

  mark_vector(lambda->constants);
}

lambda_t *make_lambda( void )
{
  return (lambda_t*)make_object(&LambdaType);
}

value_t get_const( lambda_t *lambda, size_t n )
{
  return lambda->constants[n];
}

static inline bool compare_consts( value_t x, value_t y )
{
  return x == y;
}

size_t put_const( lambda_t *lambda, value_t x )
{
  size_t i;
  
  for ( i=0; i<vector_len(lambda->constants); i++ )
    {
      if ( compare_consts(x, lambda->constants[i]) )
	goto end;
    }

  vector_push(&lambda->constants, x);

 end:
  return i;
}

void emit_instr( lambda_t *lambda, opcode_t op, ... )
{
  va_list va; va_start(va, op);

  uint x, y;
  
  switch ( op_argc(op) )
    {
    case 1:
      x = va_arg(va, uint);
      
      bytecode_pushn(&lambda->instructions, 2, op, x);
      break;

    case 2:
      x = va_arg(va, uint);
      y = va_arg(va, uint);

      bytecode_pushn(&lambda->instructions, 3, op, x, y);
      break;

    default:
      bytecode_push(&lambda->instructions, op);
      break;
    }

  va_end(va);
}

void finalize_lambda( lambda_t *lambda )
{
  emit_instr(lambda, op_halt);
  trim_vector(&lambda->constants);
  trim_bytecode(&lambda->instructions);
}

/* runtime */
void rl_obj_lambda_init( void )
{
  init_object(&LambdaType.type.obj);
}

void rl_obj_lambda_mark( void )
{
  mark_object(&LambdaType.type.obj);
}

/* convenience */
