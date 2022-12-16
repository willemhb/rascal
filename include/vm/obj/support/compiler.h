#ifndef rl_vm_obj_support_compiler_h
#define rl_vm_obj_support_compiler_h

#include "rascal.h"

#include "vm/obj/support/vector.h"
#include "vm/obj/support/bytecode.h"

/* commentary

   holds state during compilation */

/* C types */
typedef struct compiler_t      compiler_t;
typedef enum   compiler_type_t compiler_type_t;

enum compiler_type_t
  {
    compiler_type_lambda,
    compiler_type_script
  };

struct compiler_t
{
  compiler_t      *enclosing;
  compiler_type_t  type;
  bytecode_t       instructions;
  vector_t         constants;
  vector_t         locals;
  vector_t         exports;
};

/* globals */

/* API */

/* runtime dispatch */
void rl_vm_obj_support_compiler_init( void );
void rl_vm_obj_support_compiler_mark( void );
void rl_vm_obj_support_compiler_cleanup( void );

/* convenience */

#endif
