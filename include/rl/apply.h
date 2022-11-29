#ifndef rl_rl_apply_h
#define rl_rl_apply_h

#include "rascal.h"

/* commentary */

/* C types */

/* globals */

/* API */
value_t apply( value_t f, value_t args );

/* runtime */
void rl_rl_apply_init( void );
void rl_rl_apply_mark( void );

/* convenience */

#endif
