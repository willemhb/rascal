#ifndef rl_vm_error_h
#define rl_vm_error_h

#include "rascal.h"
#include "vm/obj/vm.h"

/* commentary

   runtime error handling/recovery defined here here. */

/* C types */

/* globals */

/* API */
int  panic( const char *fmt, ... );
bool panicking( void );
bool recover( void );

int check_argco( const char *fname, size_t expect, size_t got );
int check_argtypes( const char *fname, rl_value_t *args, size_t n_args, ... );

/* runtime */
void rl_vm_error_init( void );
void rl_vm_error_mark( void );
void rl_vm_error_cleanup( void );

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
