#include <stdarg.h>

#include "obj/stream.h"

#include "vm/error.h"
#include "vm/obj/vm.h"

/* commentary */

/* C types */

/* globals */

/* API */
void panic( const char *fmt, ... )
{
  va_list va; va_start(va, fmt);

  vfprintf(Errs, fmt, va);

  va_end(va);

  Vm.panic_mode = true;
}

bool recover( void )
{
  bool out      = Vm.panic_mode;
  Vm.panic_mode = false;

  return out;
}

/* runtime */
void rl_vm_error_init( void ) {}
void rl_vm_error_mark( void ) {}

/* convenience */
