#ifndef rl_def_opcodes_h
#define rl_def_opcodes_h

#include "common.h"

/* C types */
typedef enum opcode_t opcode_t;

enum opcode_t {
  op_begin,  // normal entry point
  op_halt,
  op_noop,
  op_pop,

  // load/store
  op_load_const,
  op_load_global,

  // function calls
  op_invoke,

  // jumps
  op_jump,

  // exceptions
  op_save_prompt,
  op_restore_prompt,

  /* not an opcode */
  num_opcodes
};

/* API */
size_t op_argc(opcode_t opcode);
char  *op_name(opcode_t opcode);
void   dis_op(ushort *instr, size_t *offset);

#endif
