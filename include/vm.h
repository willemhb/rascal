#ifndef rl_vm_h
#define rl_vm_h

#include "val.h"

/* C types */
struct vm_t {
  // error state
  bool  panic_mode;
  val_t error;

  // execution state
  vals_t   *stack;
  int pc, cp;         // program counter, continuation pointer
  module_t  program;
};

/* globals */
extern struct vm_t Vm;

/* API */
void   reset_vm(vm_t *vm);
size_t push(val_t x);
val_t  pop(void);
val_t  popn(size_t n);
val_t  peek(int n);
val_t *peep(int n);
val_t  poke(int n, val_t x);

#endif
