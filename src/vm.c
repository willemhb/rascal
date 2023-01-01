#include "vm.h"

#include "arr.h"

#include "list.h"

#include "prin.h"

#include "util/ios.h"

/* globals */
struct vm_t Vm;

/* API */
/* external */
void trim_stack(vm_t *vm, size_t n);
void trim_vals(vm_t *vm, size_t n);

void reset_vm(vm_t *vm) {
  vm->panic_mode=false;
  vm->error     =NUL;
  vm->pc        =  0;
  vm->bp        =  0;
  vm->cp        = -1;
  vm->program   = NULL;

  trim_stack(vm, 0);
  trim_vals(vm, 0);
}

void show_stack(void) {
  printf("current stack (sp=%zu):\n\n", Vm.stack->len);

  for (size_t i=Vm.stack->len; i > 0; i--) {
    printf("(%.4zu) ", i-1);
    prinln(peek(i-1));
  }

  newline();
}

void show_vals(void) {
  printf("current vals (sp=%zu):\n\n", Vm.vals->len);

  for (size_t i=Vm.vals->len; i > 0; i--) {
    printf("(%.4zu)", i-1);
    prinln(vals_ref(Vm.vals, i));
  }

  newline();
}

void save_frame(void) {
  
}

size_t push(val_t x) {
  size_t out = vals_push(Vm.stack, x)-1;
  return out;
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
