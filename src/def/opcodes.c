#include <stdio.h>

#include "def/opcodes.h"

/* convenience */
size_t op_argc(opcode_t op) {
  switch (op) {
  case op_invoke:
  case op_load_const:
  case op_load_global:
    return 1;

  default:
    return 0;
  }
}

char *op_name(opcode_t op) {
  switch (op) {
  case op_begin:       return "#begin";
  case op_halt:        return "#halt";
  case op_noop:        return "#no-op";
  case op_pop:         return "#pop";
  case op_load_const:  return "#load-const";
  case op_load_global: return "#load-global";
  case op_invoke:      return "#invoke";
  default:             return "#unknown-op";
  }
}

void dis_op(ushort *instr, size_t *offset) {
  size_t orig_offset = *offset;
  opcode_t op = instr[(*offset)++];

  printf("    (%.4zu) %-12s", orig_offset, op_name(op));

  size_t argc = op_argc(op);

  if (argc > 0)
    printf(" %.4x", instr[(*offset)++]);

  else
    printf(" ----");

  if (argc > 1)
    printf(" %.4x", instr[(*offset)++]);

  else
    printf(" ----");

  if (argc > 2)
    printf(" %.4x", instr[(*offset)++]);

  else
    printf(" ----");

  printf("\n");
}
