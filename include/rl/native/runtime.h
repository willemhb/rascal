#ifndef rl_rl_native_h
#define rl_rl_native_h

#include "rl/native/arithmetic.h"
#include "rl/native/interpreter.h"
#include "rl/native/objects.h"
#include "rl/native/builtins.h"

/* commentary */

/* C types */

/* globals */

/* API */

/* runtime */
void rl_rl_native_init( void );
void rl_rl_native_mark( void );
void rl_rl_native_cleanup( void );

/* convenience */

#endif
