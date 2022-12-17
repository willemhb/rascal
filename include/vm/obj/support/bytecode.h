#ifndef rl_vm_obj_support_bytecode_h
#define rl_vm_obj_support_bytecode_h

#include "rascal.h"

/* commentary

   simple array type for reprsenting Rascal instruction sequences. */

/* instantiations */
#include "tpl/decl/array.h"

ARRAY(instructions, ushort);
ARRAY(constants, rl_value_t);

/* C types */
typedef struct bytecode_t bytecode_t;

struct bytecode_t {
  instructions_t instructions;
  constants_t constants;
};

/* globals */

/* API */
ushort fetch_instruction( bytecode_t *self, size_t *pc, ushort *args );
rl_value_t get_constant( bytecode_t *self, size_t offset );

/* runtime */
void rl_vm_obj_support_bytecode_init( void );
void rl_vm_obj_support_bytecode_mark( void );
void rl_vm_obj_support_bytecode_cleanup( void );

/* convenience */

#endif
