#ifndef rascal_vm_h
#define rascal_vm_h

#include "common.h"
#include "memory.h"
#include "interpreter.h"
#include "environment.h"
#include "scanner.h"
#include "parser.h"


// global interpreter state
typedef struct {
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
} Vm;

extern Vm vm;

// forward declarations
void   initVm(Vm* vm);
void   freeVm(Vm* vm);

#endif
