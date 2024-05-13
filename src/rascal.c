#include "runtime.h"
#include "lang.h"
#include "interpreter.h"
#include "value.h"
#include "util.h"

/* Initialization. */
static void init_rascal_vm(void) {
  init_heap_state(&Heap);
  init_interpreter_state(&Interpreter);
  init_reader_state(&Reader);
  init_compiler_state(&Compiler);
}

static void free_rascal_vm(void) {
  free_heap_state(&Heap);
  free_interpreter_state(&Interpreter);
  free_reader_state(&Reader);
  free_compiler_state(&Compiler);
}

int main(const int argc, const char* argv[argc]) {
  (void)argv;

  init_rascal_vm();
  free_rascal_vm();

  return 0;
}
