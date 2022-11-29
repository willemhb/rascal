#ifndef rl_rl_exec_h
#define rl_rl_exec_h

#include "rascal.h"

/* commentary */

/* C types */

/* globals */

/* API */
value_t exec( lambda_t *code );

/* runtime */
void rl_rl_exec_init( void );
void rl_rl_exec_mark( void );

/* convenience */

#endif
