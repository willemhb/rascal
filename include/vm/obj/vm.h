#ifndef rl_vm_obj_vm_h
#define rl_vm_obj_vm_h

#include "rascal.h"

/* commentary */

/* C types */
typedef struct vm_t
{
  bool        panicking;  // panicking?
  control_t  *executing;
  namespc_t  *toplevel_names;
  envt_t     *toplevel_binds;
} vm_t;

/* globals */
extern vm_t Vm;

/* API */

/* runtime */
void rl_vm_obj_vm_init( void );
void rl_vm_obj_vm_mark( void );
void rl_vm_obj_vm_cleanup( void );

/* convenience */
value_t   get_toplevel_var( symbol_t *name );
value_t   set_toplevel_var( symbol_t *name, value_t value );
int       def_toplevel_var( symbol_t *name, value_t value );

lambda_t *get_toplevel_mac( symbol_t *name );
int       def_toplevel_mac( symbol_t *name, lambda_t *macro );

#endif
