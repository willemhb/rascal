#ifndef rl_vm_value_h
#define rl_vm_value_h

#include "rascal.h"

/* commentary 

   Basic value API (eg type querying, size querying, etc)
 */

/* C types */

/* globals */

/* API */
type_t *rl_typeof( value_t x );

/* utilities & convenience */
static inline bool rl_isa( value_t x, type_t *type )
{
  return rl_typeof(x) == type;
}

#endif
