#ifndef rl_vm_obj_support_compiler_h
#define rl_vm_obj_support_compiler_h

#include "rascal.h"

#include "vm/obj/support/bytecode.h"
#include "vm/obj/support/namespace.h"

/* commentary

   holds state during compilation */

/* C types */
typedef struct compiler_t compiler_t;
typedef enum compiler_phase_t compiler_phase_t;

enum compiler_phase_t {
  resolving_imports,
  compiling_macros,
  expanding_macros,
  resolving_namespace,
  resolving_exports, /* determine external visibility of names */
  compiling_unit, /* main compiler phase */
  finalizing_unit /* */
};

struct compiler_t
{
  compiler_t *enclosing;
  rl_symbol_t *name; /* name of the compiling code unit (function name, macro name, or file name) */
  rl_string_t *path; /* full path name to the compiling code unit (functions use the convention <file-path>:<function-name+>) */
  int scope_depth; /* 0 = module, 1 = toplevel function, 2+ = nested function */
  compiler_phase_t phase;
  namespace_t *namespace;
  instructions_t instructions;
  constants_t constants;
};

/* globals */

/* API */

/* runtime dispatch */
void rl_vm_obj_support_compiler_init( void );
void rl_vm_obj_support_compiler_mark( void );
void rl_vm_obj_support_compiler_cleanup( void );

/* convenience */

#endif
