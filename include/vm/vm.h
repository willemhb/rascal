#ifndef rl_vm_vm_h
#define rl_vm_vm_h

#include "rascal.h"

/* commentary */

/* C types */
typedef struct vm_t
{
  char      *error;       // error message
  bool       panic_mode;  // panicing?
  control_t *control;     // currently executing function or module
  closure_t *toplevel;    // toplevel module (at the moment just stores global bindings/namespace)
} vm_t;

/* globals */
extern vm_t Vm;

/* API */

/* runtime */
void rl_vm_vm_init( void );
void rl_vm_vm_mark( void );

/* convenience */

#endif
