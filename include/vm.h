#ifndef rascal_vm_h
#define rascal_vm_h

#include "memory.h"
#include "interpreter.h"
#include "environment.h"
#include "scanner.h"
#include "parser.h"
#include "compiler.h"


/* all the global state needed by the interpreter.
   Really just packages more specialized structs. */
struct Vm {
  // heap state
  Heap heap;

  // interpreter state
  Interpreter interpreter;

  // environment state
  Environment environment;
  
  // scanner state
  Scanner scanner;

  // parser state
  Parser parser;

  // compiler state
  Compiler compiler;
};

extern Vm vm;

// forward declarations
void   initVm(Vm* vm);
void   freeVm(Vm* vm);

#endif
