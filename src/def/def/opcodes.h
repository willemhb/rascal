#ifndef rl_def_opcodes_h
#define rl_def_opcodes_h

#include "common.h"

/* C types */
typedef enum opcode_t opcode_t;

enum opcode_t {
  // miscellaneous
  op_begin,
  op_halt,
  op_noop,

  // stack manipulation
  op_push,
  op_pop,
  op_dup,

  // environment manipulation
  op_bind_args,      // #bind-args <n>
  op_bind_local,     // #bind-local
  op_open_upvalue,   // #open-upvalue <i>
  op_close_upvalue,  // #close-upvalue

  // load/store
  op_load_nul,
  op_load_true,
  op_load_false,
  op_load_small_zero,
  op_load_small_one,
  op_load_small_16,
  op_load_const,
  op_load_global,
  op_store_global,
  op_load_fast,
  op_store_fast,
  op_load_upval,
  op_store_upval,

  // function calls
  op_invoke,
  op_return,

  // jumps
  op_jump,
  op_jump_true,
  op_jump_false,

  // frame manipulation
  op_save,
  op_unsave,
  op_restore,

  // error handling
  op_try,
  op_catch,
  op_panic,

  /* not an opcode */
  num_opcodes
};

/* API */
size_t op_argc(opcode_t opcode);
char  *op_name(opcode_t opcode);
void   dis_op(ushort *instr, size_t *offset);

#endif
