#include <stdio.h>

#include "def/opcodes.h"


/* convenience */
size_t op_argc(opcode_t op) {
  switch (op) {
  default: return 0;
  }
}

char *op_name(opcode_t op) {
  switch (op) {
  case op_halt: return "#halt";
  case op_noop: return "#no-op";
  default:      return "#unknown-op";
  }
}

void dis_op(ushort *instr, size_t *offset) {
  size_t orig_offset = *offset;
  opcode_t op = instr[(*offset)++];

  printf("(%.4zu) %s", orig_offset, op_name(op));

  size_t argc = op_argc(op);

  if (argc > 0)
    printf(" %.4x", instr[(*offset)++]);

  if (argc > 1)
    printf(" %.4x", instr[(*offset)++]);

  if (argc > 2)
    printf(" %.4x", instr[(*offset)++]);

  printf("\n");
}
