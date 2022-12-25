#ifndef rl_def_opcodes_h
#define rl_def_opcodes_h

#include "common.h"

/* C types */
typedef enum opcode_t opcode_t;

enum opcode_t {
  op_halt,
  op_noop,

  /* not an opcode */
  num_opcodes
};

/* API */
size_t op_argc(opcode_t opcode);
char  *op_name(opcode_t opcode);
void   dis_op(ushort *instr, size_t *offset);

#endif
