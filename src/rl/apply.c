#include "vm/value.h"

#include "rl/apply.h"

/* commentary */

/* C types */

/* globals */

/* API */
value_t apply( value_t fn, value_t args )
{
  (void)fn;
  (void)args;

  return NUL;
}

/* runtime */
void rl_rl_apply_init( void ) {}
void rl_rl_apply_mark( void ) {}

/* convenience */
