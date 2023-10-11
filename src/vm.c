#include "vm.h"

void initVm(Vm* vm) {
  initHeap(&vm->heap);
  initEnvironment(&vm->environment);
  initReader(&vm->reader);
  initCompiler(&vm->compiler, NOTHING_VAL);
  initInterpreter(&vm->interpreter, NULL);
}

void freeVm(Vm* vm) {
  freeHeap(&vm->heap);
  freeEnvironment(&vm->environment);
  freeReader(&vm->reader);
  freeCompiler(&vm->compiler);
  freeInterpreter(&vm->interpreter);
}
