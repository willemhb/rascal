#ifndef rl_rl_load_h
#define rl_rl_load_h

#include "rascal.h"

/* commentary */

/* C types */

/* globals */

/* API */
value_t load( const string_t fname );

/* runtime dispatch */
void rl_rl_load_init( void );
void rl_rl_load_mark( void );
void rl_rl_load_cleanup( void );

/* convenience */

#endif
