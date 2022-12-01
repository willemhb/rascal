#include "obj/symbol.h"
#include "obj/cons.h"

#include "vm/error.h"
#include "vm/object.h"

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
    return get_symbol_bind(as_symbol(x));

  else if ( is_cons(x) )
    {
      lambda_t *compiled = compile(x);
      value_t   value    = exec(compiled);
      return value;
    }

  else
      return x;
}

bool is_literal( value_t x )
{
  return !(is_symbol(x) || is_cons(x));
}

/* runtime */
void rl_rl_eval_init( void )    {}
void rl_rl_eval_mark( void )    {}
void rl_rl_eval_cleanup( void ) {}

/* convenience */
