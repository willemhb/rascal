#include "vm/value.h"

#include "rl/exec.h"

/* commentary */

/* C types */

/* globals */

/* API */
value_t exec( lambda_t *code )
{
  (void)code;

  return NUL;
}

/* runtime */
void rl_vm_exec_init( void ) {}
void rl_vm_exec_mark( void ) {}

/* convenience */
