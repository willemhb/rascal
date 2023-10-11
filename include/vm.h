#ifndef rascal_vm_h
#define rascal_vm_h

#include "memory.h"
#include "environment.h"
#include "reader.h"
#include "compiler.h"
#include "interpreter.h"

/* all the global state needed by the interpreter.
   Really just packages more specialized structs. */
struct Vm {
  // heap state
  Heap heap;

  // environment state
  Environment environment;

  // reader state
  Reader reader;

  // compiler state
  Compiler compiler;

  // interpreter state
  Interpreter interpreter;
};

// globals
extern Vm vm;

// external API
void   initVm(Vm* vm);
void   freeVm(Vm* vm);

Value  exec(Vm* vm, Chunk* code);
Value  eval(Vm* vm, Value val);
void   repl(Vm* vm);

#endif
