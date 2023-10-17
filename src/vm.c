#include "util/hashing.h"
#include "util/number.h"

#include "runtime.h"
#include "array.h"
#include "memory.h"
#include "environment.h"
#include "read.h"
#include "compile.h"
#include "eval.h"
#include "vm.h"
#include "equal.h"

// external API
void initVm(Vm* vm) {
  initHeap(vm);
  initEnvt(vm);
  initReader(vm);
  initCompiler(vm);
  initInterpreter(vm);
  initRuntime(vm);
}

void freeVm(Vm* vm) {
  freeRuntime(vm);
  freeInterpreter(vm);
  freeCompiler(vm);
  freeReader(vm);
  freeEnvt(vm);
  freeHeap(vm);
}

void syncVm(Vm* vm) {
  syncReader(vm);
  syncCompiler(vm);
  syncInterpreter(vm);
}

size_t push(Value x) {
  assert(RlVm.exec.sp < N_STACK);
  size_t out = RlVm.exec.sp;
  RlVm.stackBase[RlVm.exec.sp++] = x;
  return out;
}

Value pop(void) {
  assert(RlVm.exec.sp > 0);
  return RlVm.stackBase[--RlVm.exec.sp];
}

size_t pushn(size_t n) {
  assert(RlVm.exec.sp + n < N_STACK);
  size_t out = RlVm.exec.sp;
  RlVm.exec.sp += n;

  return out;
}

Value popn(size_t n) {
  assert(n <= RlVm.exec.sp);
  Value out = RlVm.stackBase[RlVm.exec.sp-n];
  RlVm.exec.sp -= n;
  return out;
}

Value* peek(int i) {
  if (i < 0)
    i += RlVm.exec.sp;

  assert(i >= 0 && (size_t)i < RlVm.exec.sp);

  return RlVm.stackBase+i;
}

size_t save(size_t n, ...) {
  Obj* buf[n];
  va_list va;
  va_start(va, n);
  size_t i, j;

  for (i=0, j=0; i<n; i++) {
    Value v = va_arg(va, Value);

    if (IS_OBJ(v) && AS(Obj, v) != NULL && !AS(Obj, v)->black)
      buf[j] = AS(Obj, v);
  }

  va_end(va);

  if (i > 0) {
    markObjs(i, (void**)buf);
    objectsWrite(&RlVm.heap.grays, i, buf);
  }

  return i;
}

void unsave(size_t n) {
  n = max(n, RlVm.heap.grays.cnt); // 

  if (n > 0) {
    unmarkObjs(n, (void**)RlVm.heap.grays.data);
    objectsPopN(&RlVm.heap.grays, n);
  }
}
