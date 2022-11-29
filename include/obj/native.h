#ifndef rl_obj_native_h
#define rl_obj_native_h

#include "rascal.h"

/* commentary */

/* C types */

/* globals */
extern datatype_t NativeType;

/* API */

/* runtime */
void rl_obj_native_init( void );
void rl_obj_native_mark( void );

/* convenience */
#define is_nul( x ) ((x)==NUL)
#define as_nul( x ) (NULL)

#endif
