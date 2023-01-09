#include <stdio.h>

#include "def/opcodes.h"

/* convenience */
size_t op_argc(opcode_t op) {
  switch (op) {
  case op_load_small_16 ... op_store_local:
  case op_invoke:
  case op_jump ... op_jump_false:
    return 1;

  case op_load_nonlocal ... op_store_nonlocal:
    return 2;

  default:
    return 0;
  }
}

char *op_name(opcode_t op) {
  switch (op) {
    // misc
  case op_begin:           return "#begin";
  case op_halt:            return "#halt";
  case op_noop:            return "#no-op";

    // stack manipulation
  case op_push:            return "#push";
  case op_pop:             return "#pop";
  case op_dup:             return "#dup";

    // load/store
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
  case op_load_nonlocal:   return "#load-nonlocal";
  case op_store_nonlocal:  return "#store-nonlocal";

    // function calls
  case op_invoke:          return "#invoke";

    // jumps
  case op_jump:            return "#jump";
  case op_jump_true:       return "#jump-if-true";
  case op_jump_false:      return "#jump-if-false";

    // frame manipulation
  case op_save_frame:      return "#save-frame";
  case op_restore_frame:   return "#restore-frame";
  case op_save_prompt:     return "#save-prompt";
  case op_restore_prompt:  return "#restore-prompt";
  case op_discard_prompt:  return "#discard-prompt";

    // fallback
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
