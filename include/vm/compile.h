#ifndef rl_vm_compile_h
#define rl_vm_compile_h

#include "vm/context.h"

/* Global state and APIs for the compiler. */
/* Globals */
extern Value     CompilerValues[];
extern CompFrame CompilerFrames[];

/* External API */
Value* peek_compiler_val(int n);
void   push_compiler_val(Value val);
Value  pop_compiler_val(void);

CompFrame* peek_compiler_frame(int n);
void       push_compiler_frame(void);
void       pop_compiler_frame(void);

void vm_mark_compiler(void);

/* Initialization. */
void vm_init_compiler(void);

#endif
