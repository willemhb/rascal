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
Value* write_interp_vals(Value* val, size_t n);
Value  pop_interp_val(void);
void   pop_interp_vals(size_t n);

ExecFrame* peek_interp_frame(int n);
void       push_interp_frame(void);
ExecFrame* write_interp_frames(ExecFrame* f, size_t n);
void       pop_interp_frame(void);
void       pop_interp_frames(size_t n);

void mark_exec_frame(ExecFrame* frame);
void mark_exec_frames(ExecFrame* frames, size_t n);
void vm_mark_interpreter(void);

/* Initialization. */
void vm_init_interpreter(void);

#endif
