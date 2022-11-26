#ifndef rl_vm_vm_h
#define rl_vm_vm_h

#include "rascal.h"

/* commentary */

/* C types */
typedef struct vm_t
{
  bool       panic_mode;  // panicing?
  control_t  executing;
} vm_t;

/* globals */
extern vm_t Vm;

/* API */

/* runtime */
void rl_vm_vm_init( void );

/* convenience */

#endif
