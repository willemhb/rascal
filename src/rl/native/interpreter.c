
#include "vm/error.h"
#include "vm/value.h"

#include "obj/lambda.h"
#include "obj/closure.h"

#include "rl/eval.h"
#include "rl/exec.h"
#include "rl/compile.h"
#include "rl/read.h"
#include "rl/prin.h"

#include "rl/native/interpreter.h"

/* commentary */

/* C types */

/* globals */

/* API */
value_t native_eval( value_t *args, int nargs )
{
  int status;

  if ( (status=check_argco("eval", 1, nargs)) < 0 )
    return NUL;

  return eval(args[0]);
}

value_t native_exec( value_t *args, int nargs )
{
  int status;

  if ( (status=check_argco("exec", 1, nargs)) < 0 )
    return NUL;

  if ( (status=check_argtypes("exec", args, 1, &LambdaType)) < 0 )
    return NUL;

  return exec(as_lambda(args[0]));
}

value_t native_apply( value_t *args, int nargs )
{
  
}

/* runtime dispatch */
void rl_rl_native_interpreter_init( void ) {}
void rl_rl_native_interpreter_mark( void ) {}
void rl_rl_native_interpreter_cleanup( void ) {}

