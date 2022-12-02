#ifndef rl_rl_native_arithmetic_h
#define rl_rl_native_arithmetic_h

#include "rascal.h"

/* commentary */

/* C types */

/* globals */

/* API */
value_t native_add( value_t *args, int nargs );
value_t native_sub( value_t *args, int nargs );
value_t native_mul( value_t *args, int nargs );
value_t native_div( value_t *args, int nargs );
value_t native_eq( value_t *args, int nargs );
value_t native_lt( value_t *args, int nargs );
value_t native_gt( value_t *args, int nargs );
value_t native_leq( value_t *args, int nargs );
value_t native_geq( value_t *args, int nargs );
value_t native_neq( value_t *args, int nargs );

/* runtime */
void rl_rl_native_arithmetic_init( void );
void rl_rl_native_arithmetic_mark( void );
void rl_rl_native_arithmetic_cleanup( void );

#endif
