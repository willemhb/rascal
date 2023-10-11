#ifndef rascal_vm_h
#define rascal_vm_h

#include "memory.h"
#include "runtime.h"
#include "environment.h"
#include "reader.h"
#include "compiler.h"
#include "interpreter.h"

/* all the global state needed by the interpreter.
   Really just packages more specialized structs. */
struct Vm {
  // heap state
  Heap heap;

  // exception context
  Context context;

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
extern Vm RlVm;

// external API
// vm accessors

void initVm(Vm* vm);
void freeVm(Vm* vm);
void syncVm(Vm* vm);

#endif
