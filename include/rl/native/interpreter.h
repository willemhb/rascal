#ifndef rl_rl_native_interpreter_h
#define rl_rl_native_interpreter_h

#include "rascal.h"

/* commentary */

/* C types */

/* globals */

/* API */
value_t native_eval( value_t *args, int nargs );
value_t native_apply( value_t *args, int nargs );
value_t native_exec( value_t *args, int nargs );
value_t native_comp( value_t *args, int nargs );
value_t native_read( value_t *args, int nargs );
value_t native_prin( value_t *args, int nargs );
value_t native_repl( value_t *args, int nargs );

/* runtime dispatch */
void rl_rl_native_interpreter_init( void );
void rl_rl_native_interpreter_mark( void );
void rl_rl_native_interpreter_cleanup( void );

/* convenience */

#endif
