#ifndef rl_vm_obj_vm_h
#define rl_vm_obj_vm_h

#include "rascal.h"

/* commentary */

/* C types */
typedef struct vm_t
{
  /* global state */
  bool      panicking;      // panicking?
  ns_t     *toplevel;       // toplevel namespace (stores variables created in REPL and loaded modules).
  upval_t  *open;

  /* execution state */
  stack_t  *stack;          // arguments & locals
  module_t *module;         // currently executing module
  module_t *function;       // currently executing function
  size_t    bp, ip, pp, cp; // base pointer, instruction pointer, prompt pointer, continue pointer
} vm_t;

/* globals */
extern vm_t Vm;

/* API */
/* basic environment utilities */

/* runtime */
void rl_vm_obj_vm_init( void );
void rl_vm_obj_vm_mark( void );
void rl_vm_obj_vm_cleanup( void );

/* convenience */

#endif
