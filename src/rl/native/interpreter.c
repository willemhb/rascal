
#include "vm/error.h"
#include "vm/value.h"

#include "obj/lambda.h"
#include "obj/closure.h"
#include "obj/native.h"

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
  int status;

  if ( (status=check_argco("apply", 2, nargs)) < 0 )
    return NUL;

  
}

/* runtime dispatch */
void rl_rl_native_interpreter_init( void )
{
  define_native("eval", native_eval);
  define_native("apply", native_apply);
  define_native("exec", native_exec);
  define_native("comp", native_comp);
  define_native("read", native_read);
  define_native("prin", native_prin);
  define_native("repl", native_repl);
}

void rl_rl_native_interpreter_mark( void ) {}
void rl_rl_native_interpreter_cleanup( void ) {}

