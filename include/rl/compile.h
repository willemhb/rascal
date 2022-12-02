#ifndef rl_rl_compile_h
#define rl_rl_compile_h

#include "rascal.h"

/* commentary */

/* C types */

/* globals */

/* API */
lambda_t *compile( value_t x );
lambda_t *compile_file( const char *fname );

/* runtime */
void rl_rl_compile_init( void );
void rl_rl_compile_mark( void );
void rl_rl_compile_cleanup( void );

/* convenience */

#endif
