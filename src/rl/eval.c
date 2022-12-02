#include "obj/symbol.h"
#include "obj/cons.h"

#include "vm/error.h"
#include "vm/object.h"
#include "vm/obj/vm.h"

#include "rl/compile.h"
#include "rl/exec.h"
#include "rl/eval.h"

/* commentary

   This crude definition of eval is included mostly for convenience,
   and to simplify the implementation of repl(). */

/* C types */

/* globals */

/* API */
value_t eval( value_t x )
{
  if ( is_symbol(x) )
    {
      value_t out = toplevel_lookup(as_symbol(x));

      if ( panicking() )
	return NUL;

      return out;
    }

  else if ( is_cons(x) )
    {
      lambda_t *compiled = compile(x);

      if ( panicking() )
	return NUL;
      
      value_t value = exec(compiled);

      if ( panicking() )
	return NUL;
      return value;
    }

  else
      return x;
}

/* runtime */
void rl_rl_eval_init( void )    {}
void rl_rl_eval_mark( void )    {}
void rl_rl_eval_cleanup( void ) {}

/* convenience */
