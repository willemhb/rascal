#include <stdarg.h>

#include "obj/stream.h"
#include "obj/type.h"

#include "vm/error.h"
#include "vm/obj/vm.h"

/* commentary */

/* C types */

/* globals */

/* API */
int vpanic( const char *fmt, va_list va );

int panic( const char *fmt, ... )
{
  va_list va; va_start(va, fmt);
  int status = vpanic(fmt, va);
  va_end(va);

  return status;
}

int vpanic( const char *fmt, va_list va )
{
  vfprintf(Errs, fmt, va);
  Vm.panicking = true;

  return -1;
}

bool panicking( void )
{
  return Vm.panicking;
}

bool recover( void )
{
  bool out      = Vm.panicking;
  Vm.panicking  = false;

  return out;
}

int check_argco( const char *fname, size_t expect, size_t got )
{
  int status = 0 - panicking();
  if ( status < 0 )
    return status;

  if ( expect != got )
    return panic("%s wanted %zu inputs, got %zu.\n", fname, expect, got);

  return got;
}

int check_argtypes( const char *fname, value_t *args, size_t n_args, ... )
{
  static const char *fmt = "%s wanted a %s for its %zu-th input, but got a %s.\n";

  int status = 0 - panicking();
  
  if ( status < 0 )
    return status;

  va_list va; va_start(va, n_args);

  for ( size_t i=0; i<n_args; i++ )
    {
      datatype_t *type = va_arg(va, datatype_t*);

      if ( type == NULL )
	continue;

      if ( !rl_isa(args[i], type) )
	{
	  return panic(fmt,
		       fname,
		       get_datatype_name(type),
		       i,
		       get_datatype_name(rl_typeof(args[i])));
	}
    }

  va_end(va);

  return n_args;
}

/* runtime */
void rl_vm_error_init( void )    {}
void rl_vm_error_mark( void )    {}
void rl_vm_error_cleanup( void ) {}

/* convenience */
