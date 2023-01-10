#ifndef rl_vm_h
#define rl_vm_h

#include "obj.h"

/* C types */
struct vm_t {
  // error handling state
  val_t panic_cause;
  module_t hl;                  // catch handler (saved)
  bool panic_mode;
  int cp;                       // catch pointer (saved)

  // execution & environment state
  int fc;                       // frame count (saved)
  int pc;                       // program counter (saved)
  module_t pr;                  // executing program (saved)

  // stacks
  vals_t *stack, *envt, *frame; // function arguments, locals, call frames

  // miscellaneous
  val_t open_upvals;            // list of 
};

/* globals */
extern struct vm_t Vm;

/* API */
void   reset_vm(vm_t *vm);
void   show_stack(void);
void   show_frame(void);
void   show_envt(void);
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
