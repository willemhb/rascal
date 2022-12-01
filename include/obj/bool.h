#ifndef rl_obj_bool_h
#define rl_obj_bool_h

#include "vm/value.h"

/* commentary */

/* C types */

/* globals */
extern datatype_t BoolType;

/* API */
bool as_cbool( value_t x );

/* runtime */
void rl_obj_bool_init( void );
void rl_obj_bool_mark( void );
void rl_obj_bool_cleanup( void );

/* convenience */
#define is_bool( x ) (tagof(x)==BOOL)
#define as_bool( x )  as_cbool(x)

#endif
