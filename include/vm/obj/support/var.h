#ifndef rl_vm_obj_support_var_h
#define rl_vm_obj_support_var_h

#include "rascal.h"

/* commentary */

/* C types */
typedef struct var_t var_t;
typedef struct namespace_t namespace_t;
typedef enum var_phase_t var_phase_t;
typedef struct type_t type_t;

enum var_phase_t {
  expand_phase,
  runtime_phase
};

struct var_t {
  rl_symbol_t *name; // 
  namespace_t *namespace;
  rl_cons_t *metadata;
  type_t *type; // type constraint on the variable

  ushort captured;
  ushort exported;
  var_phase_t phase;

  size_t offset;
  rl_function_t *function; // 
};

/* globals */

/* API */

/* runtime dispatch */
void rl_vm_obj_support_var_init( void );
void rl_vm_obj_support_var_mark( void );
void rl_vm_obj_support_var_cleanup( void );

/* convenience */

#endif
