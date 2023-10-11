#ifndef rascal_interpreter_h
#define rascal_interpreter_h

#include "common.h"
#include "opcodes.h"
#include "object.h"

// C types
struct Interpreter {
  Value*    sp;       // stack pointer
  uint16_t* ip;       // instruction pointer
  Chunk*    code;     // executing function
};

// globals
#define N_STACK 8192
extern Value TheStack[N_STACK];

// external API
void initInterpreter(Interpreter* interpreter, Chunk* code);
void freeInterpreter(Interpreter* interpreter);

Value  exec(Vm* vm, Chunk* code);
Value  eval(Vm* vm, Value val);
void   repl(Vm* vm);

#endif
