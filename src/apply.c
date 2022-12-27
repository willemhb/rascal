#include "apply.h"

#include "vm.h"

#include "def/opcodes.h"

/* API */
extern val_t exec_at(module_t module, opcode_t entry, uint argx);

val_t invoke(val_t fn, size_t nargs, val_t *args) {
  push(fn);
  appendto_vals(Vm.stack, args, nargs);
  
  val_t out = exec_at(NULL, op_invoke, nargs);

  return out;
}
