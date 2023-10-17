#ifndef rascal_eval_h
#define rascal_eval_h

#include "opcodes.h"
#include "vm.h"

// C types
void initInterpreter(Vm* vm);
void freeInterpreter(Vm* vm);
void startInterpreter(Vm* vm, Closure* code);
void resetInterpreter(Vm* vm);
void syncInterpreter(Vm* vm);

Value  load(char* path);
Value  exec(void* code);
Value  eval(Value val);
void   repl(void);

#endif
