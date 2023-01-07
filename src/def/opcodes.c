#include <stdio.h>

#include "def/opcodes.h"

/* convenience */
size_t op_argc(opcode_t op) {
  switch (op) {
  case op_load_small_16 ... op_invoke:
  case op_jump ... op_jump_false:
    return 1;

  default:
    return 0;
  }
}

char *op_name(opcode_t op) {
  switch (op) {
  case op_begin:           return "#begin";
  case op_halt:            return "#halt";
  case op_noop:            return "#no-op";
  case op_pop:             return "#pop";
  case op_load_nul:        return "#load-nul";
  case op_load_true:       return "#load-true";
  case op_load_false:      return "#load-false";
  case op_load_small_zero: return "#load-small-0";
  case op_load_small_one:  return "#load-small-1";
  case op_load_small_16:   return "#load-small-16";
  case op_load_const:      return "#load-const";
  case op_load_global:     return "#load-global";
  case op_store_global:    return "#store-global";
  case op_load_local:      return "#load-local";
  case op_store_local:     return "#store-local";
  case op_invoke:          return "#invoke";
  case op_return:          return "#return";
  case op_jump:            return "#jump";
  case op_jump_true:       return "#jump-if-true";
  case op_jump_false:      return "#jump-if-false";
  case op_save_prompt:     return "#save-prompt";
  case op_restore_prompt:  return "#restore-prompt";
  case op_discard_prompt:  return "#discard-prompt";
  default:                 return "#unknown-op";
  }
}

void dis_op(ushort *instr, size_t *offset) {
  size_t orig_offset = *offset;
  opcode_t op = instr[(*offset)++];

  printf("    (%.4zu) %-15s", orig_offset, op_name(op));

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
