#ifndef rl_vm_exec_h
#define rl_vm_exec_h

#include "rascal.h"

/* commentary */

/* C types */

/* globals */

/* API */
value_t exec( lambda_t *code );
value_t eval( value_t x );

/* runtime */
void rl_vm_exec_init( void );

/* convenience */

#endif
