#include "vm.h"

#include "small.h"
#include "module.h"

#include "arr.h"
#include "list.h"

#include "prin.h"

#include "util/ios.h"

/* globals */
struct vm_t Vm;

/* API */
/* external */
void reset_vm(vm_t *vm) {
  vm->panic_mode= false;
  vm->error     = NUL;
  vm->pc        =  0;
  vm->bp        =  0;
  vm->cp        =  0;
  vm->program   = NULL;

  trim_stack(0);
  trim_frame(0);
}

void show_stack(void) {
  printf("stack (sp=%zu):\n\n", Vm.stack->len);

  for (size_t i=Vm.stack->len; i > 0; i--) {
    printf("(%.4zu)", i-1);
    prinln(peek(i-1));
  }

  newline();
}

void show_frame(void) {
  printf("frames (fp=%zu):\n\n", Vm.frame->len);

  for (size_t i=Vm.stack->len; i > 0; i -= 4) {
    val_t *frame = alist_at(Vm.stack, i-4, val_t);

    printf("(%.4zu) sp=", i-1); prinln(frame[3]);
    printf("(%.4zu) cp=", i-2); prinln(frame[2]);
    printf("(%.4zu) bp=", i-3); prinln(frame[1]);
    printf("(%.4zu) pc=", i-4); prinln(frame[0]);
  }

  newline();
}

void trim_stack(int n) {
  vals_trim(Vm.stack, n);
}

void trim_frame(int n) {
  vals_trim(Vm.frame, n);
}

/* stack manipulation helpers */
int push(val_t x) {
  assert(Vm.stack->len < INT32_MAX);
  int out = vals_push(Vm.stack, x)-1;
  return out;
}

int pushn(int n) {
  assert((size_t)n + Vm.stack->len <= INT32_MAX);

  int out = Vm.stack->len; vals_trim(Vm.stack, out+n);

  return out;
}

val_t pop(void) {
  return vals_pop(Vm.stack);
}

val_t popn(int n) {
  assert(n > 0);
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
    .panic_mode= false,
    .error     = NUL,
    .stack     = make_vals(0, NULL),
    .frame     = make_vals(0, NULL),
    .pc        = 0,
    .cp        = 0,
    .bp        = 0,
    .program   = NULL
  };
}
