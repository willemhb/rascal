#ifndef rl_vm_h
#define rl_vm_h

#include "val.h"

/* C types */
struct vm_t {
  // error state
  bool  panic_mode;
  val_t error;

  // execution state
  vals_t *stack, *frame; // arguments and locals, call frames
  int pc, bp, cp;        // program counter, base pointer, continuation pointer

  module_t  program;  // executing program
};

/* globals */
extern struct vm_t Vm;

/* API */
void   reset_vm(vm_t *vm);
void   show_stack(void);
void   show_frame(void);
void   trim_stack(int n);
void   trim_frame(int n);

int    push(val_t x);
int    pushn(int n);
val_t  pop(void);
val_t  popn(int n);
val_t  peek(int n);
val_t *peep(int n);
val_t  poke(int n, val_t x);

#endif
