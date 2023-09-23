#include "vm.h"

Vm vm;

void initVm(Vm* vm) {
  initHeap(&vm->heap);
  initScanner(&vm->scanner, NULL);
}

void freeVm(Vm* vm) {
  freeHeap(&vm->heap);
  freeScanner(&vm->scanner);
}
