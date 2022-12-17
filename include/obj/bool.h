#ifndef rl_obj_bool_h
#define rl_obj_bool_h

#include "vm/value.h"

/* commentary */

/* C types */

/* globals */

/* API */
bool as_cbool( rl_value_t x );

/* runtime */
void rl_obj_bool_init( void );
void rl_obj_bool_mark( void );
void rl_obj_bool_cleanup( void );

/* convenience */
#define is_bool( x ) has_value_type(x, bool_value)

#endif
