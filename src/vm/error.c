#include <stdarg.h>

#include "obj/stream.h"
#include "obj/type.h"

#include "vm/error.h"
#include "vm/obj/vm.h"

/* commentary */

/* C types */

/* globals */

/* API */
void vpanic( const char *fmt, va_list va );

void panic( const char *fmt, ... )
{
  va_list va; va_start(va, fmt);
  vpanic(fmt, va);
  va_end(va);
}

void vpanic( const char *fmt, va_list va )
{
  vfprintf(Errs, fmt, va);
  Vm.panic_mode = true;
}

bool panicking( void )
{
  return Vm.panic_mode;
}

bool recover( void )
{
  bool out      = Vm.panic_mode;
  Vm.panic_mode = false;

  return out;
}

void check_argco( const char *fname, size_t expect, size_t got )
{
  if ( Vm.panic_mode )
    return;

  if ( expect != got )
    panic("%s wanted %zu inputs, got %zu.\n", fname, expect, got);
}

void check_argtypes( const char *fname, value_t *args, size_t n_args, ... )
{
  static const char *fmt = "%s wanted a %s for its %zu-th input, but got a %s.\n";
  if ( Vm.panic_mode )
    return;

  va_list va; va_start(va, n_args);

  for ( size_t i=0; i<n_args; i++ )
    {
      datatype_t *type = va_arg(va, datatype_t*);

      if ( type == NULL )
	continue;

      if ( !rl_isa(args[i], type) )
	{
	  panic(fmt, fname, type_name(type), i, type_name(rl_typeof(args[i])));
	  break;
	}
    }

  va_end(va);
}

/* runtime */
void rl_vm_error_init( void ) {}
void rl_vm_error_mark( void ) {}

/* convenience */
