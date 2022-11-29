#ifndef rl_obj_native_h
#define rl_obj_native_h

#include "rascal.h"

/* commentary

   Type for builtin functions represented by C function pointers. */

/* C types */

/* globals */
extern datatype_t NativeType;

/* API */

/* runtime */
void rl_obj_native_init( void );
void rl_obj_native_mark( void );

/* convenience */
#define is_native( x ) (tagof(x)==NATIVE)
#define as_native( x ) ((native_t)dataof(x))

#endif
