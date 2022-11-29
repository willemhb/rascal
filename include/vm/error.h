#ifndef rl_vm_error_h
#define rl_vm_error_h

#include "rascal.h"

/* commentary

   runtime error handling/recovery defined here here. */

/* C types */

/* globals */

/* API */
void panic( const char *fmt, ... );
bool recover( void );

/* runtime */
void rl_vm_error_init( void );
void rl_vm_error_mark( void );

/* convenience */

#endif
