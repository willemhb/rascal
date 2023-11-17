#ifndef rl_vm_interpreter_h
#define rl_vm_interpreter_h

#include "vm/context.h"

/* Call stack and Value stack API. */
/* Globals */
extern Value     InterpreterValues[];
extern ExecFrame InterpreterFrames[];

/* External API */
Value* peek_interp_val(int n);
void   push_interp_val(Value val);
Value  pop_interp_val(void);

ExecFrame* peek_interp_frame(int n);
void       push_interp_frame(void);
void       pop_interp_frame(void);

void vm_mark_interpreter(void);

/* Initialization. */
void vm_init_interpreter(void);

#endif
