#include "vm/context.h"

/* initialization. */
#include "vm/memory.h"
#include "vm/error.h"
#include "vm/envt.h"
#include "vm/read.h"
#include "vm/compile.h"
#include "vm/interpreter.h"

void vm_init_context(void) {
  vm_init_memory();
  vm_init_error();
  vm_init_envt();
  vm_init_reader();
  vm_init_compiler();
  vm_init_interpreter();
}
