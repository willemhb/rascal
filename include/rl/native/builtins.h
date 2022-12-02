#ifndef rl_rl_native_builtins_h
#define rl_rl_native_builtins_h

#include "rascal.h"

/* commentary

   miscellaneous other builtin functions */

/* C types */

/* globals */

/* API */
value_t native_idp( value_t *args, size_t nargs );
value_t native_isap( value_t *args, size_t nargs );
value_t native_not( value_t *args, size_t nargs );
value_t native_typeof( value_t *args, size_t nargs );
value_t native_lookup( value_t *args, size_t nargs );
value_t native_boundp( value_t *args, size_t nargs );
value_t native_panic( value_t *args, size_t nargs );
value_t native_exit( value_t *args, size_t nargs );

/* runtime dispatch */
void rl_rl_native_builtins_init( void );
void rl_rl_native_builtins_mark( void );
void rl_rl_native_builtins_cleanup( void );

/* convenience */

#endif
