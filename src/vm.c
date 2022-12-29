#include "vm.h"

#include "arr.h"

#include "list.h"

/* globals */
struct vm_t Vm;

/* API */
void reset_vm(vm_t *vm) {
  vm->panic_mode=false;
  vm->error     =NUL;
  vm->pc        = 0;
  vm->program   = NULL;
  vals_popn(vm->stack, vm->stack->len);
}

size_t push(val_t x) {
  return vals_push(Vm.stack, x);
}

val_t pop(void) {
  return vals_pop(Vm.stack);
}

val_t popn(size_t n) {
  return vals_popn(Vm.stack, n);
}

val_t peek(int n) {
  return vals_ref(Vm.stack, n);
}

val_t *peep(int n) {
  if (n < 0)
    n += Vm.stack->len;

  assert_bound(n, Vm.stack->len);
  return ((val_t*)Vm.stack->elements)+n;
}

val_t poke(int n, val_t x) {
  return vals_set(Vm.stack, n, x);
}

/* initialization */
void vm_init(void) {
  Vm = (struct vm_t) {
    .panic_mode=false,
    .error     =NUL,
    .stack     =make_vals(0, NULL),
    .pc        =0,
    .cp        =-1,
    .program   =NULL
  };
}
