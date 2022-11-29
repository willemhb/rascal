#include "vm/error.h"
#include "vm/value.h"

#include "rl/eval.h"

/* commentary */

/* C types */

/* globals */

/* API */
value_t eval( value_t x )
{
  if ( is_literal(x) )
    return x;

  else
    {
      panic("Unrecognized expression type.");
      return NUL;
    }
}

bool is_literal( value_t x )
{
  (void)x;

  return true;
}


/* runtime */
void rl_rl_eval_init( void ) {}
void rl_rl_eval_mark( void ) {}

/* convenience */
