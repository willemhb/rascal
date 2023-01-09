#ifndef rl_vm_h
#define rl_vm_h

#include "val.h"

/* C types */
struct vm_t {
  // panic mode state
  bool  panic_mode;
  val_t error;

  // execution state
  vals_t *stack, *frame; // arguments and locals, call frames
  val_t val;             // value register (most recent expression value)

  // main registers (saved in frame)
  int bp, fc, cp, pc;
  module_t pr, hl;
};

/* registers
   bp - location of first local binding
   fc - frame count (number of local bindings)
   cp - location of activation record saved by `catch`
   pc - program counter. program->code[pc] is the next instruction to execute.
   pr - program. Currently executing module object
   hl - Current error handler (no handler installed if NULL)

   activation record layout
   +----+----+----+----+----+----+
   | bp | fc | cp | pc | pr | hl |
   +----+----+----+----+----+----+
*/

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
