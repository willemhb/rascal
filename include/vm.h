#ifndef rl_vm_h
#define rl_vm_h

#include "val.h"

/* C types */
struct vm_t {
  // error state
  bool  panic_mode;
  val_t error;

  // execution state
  vals_t   *stack, *vals; // callstack, local arguments
  int pc, bp, cp;         // program counter, continuation pointer
  module_t  program;
};

/* globals */
extern struct vm_t Vm;

/* API */
void   reset_vm(vm_t *vm);
void   show_stack(void);
void   show_vals(void);

void   save_frame(void);
void   restore_frame(void);
void   save_prompt(void);
void   restore_prompt(void);
void   discard_prompt(void);

size_t push(val_t x);
val_t  pop(void);
val_t  popn(size_t n);
val_t  peek(int n);
val_t *peep(int n);
val_t  poke(int n, val_t x);


#endif
