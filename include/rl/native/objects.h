#ifndef rl_rl_native_objects_h
#define rl_rl_native_objects_h

#include "rascal.h"

/* commentary */

/* C types */

/* globals */

/* API */
value_t native_cons( value_t *args, size_t nargs );
value_t native_car( value_t *args, size_t nargs );
value_t native_cdr( value_t *args, size_t nargs );

/* runtime dispatch */
void rl_rl_native_objects_init( void );
void rl_rl_native_objects_mark( void );
void rl_rl_native_objects_cleanup( void );

/* convenience */

#endif
