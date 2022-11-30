#ifndef rl_vm_error_h
#define rl_vm_error_h

#include "rascal.h"
#include "vm/obj/vm.h"

/* commentary

   runtime error handling/recovery defined here here. */

/* C types */

/* globals */

/* API */
void panic( const char *fmt, ... );
void vpanic( const char *fmt, va_list va );
bool recover( void );

void check_argco( const char *fname, size_t expect, size_t got );
void check_argtypes( const char *fname, value_t *args, size_t n_args, ... );

/* runtime */
void rl_vm_error_init( void );
void rl_vm_error_mark( void );

/* convenience */
#define safe_call( f, sentinel, ... )		\
  ({						\
    if ( Vm.panic_mode )			\
      return sentinel;				\
    						\
    f(__VA_ARGS__);				\
  })

#define VOID_RETURN
  

#endif
