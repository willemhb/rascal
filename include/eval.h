#ifndef rascal_eval_h
#define rascal_eval_h

#include "opcodes.h"
#include "runtime.h"

// C types
void init_interpreter(Vm* vm);
void free_interpreter(Vm* vm);
void start_interpreter(Vm* vm, Closure* code);
void reset_interpreter(Vm* vm);
void sync_interpreter(Vm* vm);

Value  apply_cl(Closure* cl, size_t n);
Value  load(char* path);
Value  exec(Closure* code);
Value  eval(Value val);
void   repl(void);

#endif
