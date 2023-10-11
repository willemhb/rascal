#ifndef rascal_interpreter_h
#define rascal_interpreter_h

#include "common.h"
#include "opcodes.h"
#include "object.h"

// C types
struct Interpreter {
  Value*    sp, *vp, *ep; // stack pointer, value pointer, end pointer
  uint16_t* ip;           // instruction pointer
  Chunk*    code;         // executing function
};

// globals
#define N_STACK 8192
extern Value TheStack[N_STACK];

// external API
void  initInterpreter(Interpreter* interpreter, Value* vals, size_t nStack);
void  freeInterpreter(Interpreter* interpreter);
void  startInterpreter(Interpreter* interpreter, Chunk* code);
void  resetInterpreter(Interpreter* interpreter);
void  syncInterpreter(Interpreter* interpreter);

void  push(Vm* vm, Value value);
void  pushn(Vm* vm, size_t n);
Value pop(Vm* vm);
Value peek(Vm* vm, int i);

Value  exec(Vm* vm, Chunk* code);
Value  eval(Vm* vm, Value val);
void   repl(Vm* vm);

#endif
