
#include "vm/value.h"

/* API */
value_t value_type( rl_value_t x )
{
  if ( (x&QNAN) != QNAN )
    return real_value;

  uint out = x>>48 & 3;

  if ( !!(x&SIGNBIT) )
    out |= 4;

  return out;
}

void *toptr( rl_value_t x )
{
  if ( x == NUL )
    return NULL;

  return (void*)(x&PTRMASK);
}

/* runtime */
void rl_vm_value_init( void )    {}
void rl_vm_value_mark( void )    {}
void rl_vm_value_cleanup( void ) {}
