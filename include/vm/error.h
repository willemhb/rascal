#ifndef rl_vm_error_h
#define rl_vm_error_h

#include "rascal.h"

/* commentary

   runtime error handling/recovery defined here here. */

/* C types */

/* globals */

/* API */
void panic_( const char *file, int line, const char *func, const char *fmt, ... );
void rl_abort_( const char *file, int line, const char *func, const char *fmt, ... );
bool recover( void );

/* runtime */

/* convenience */
#define panic( fmt, ... ) panic_(__FILE__, __LINE__, __func__, fmt __VA_OPT__(,) __VA_ARGS__)
#define rl_abort( fmt, ... ) rl_abort_(__FILE__, __LINE__, __func__, fmt __VA_OPT__(,) __VA_ARGS__)

#endif
