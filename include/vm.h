#ifndef rascal_vm_h
#define rascal_vm_h

#include "memory.h"
#include "scanner.h"
#include "environment.h"

/* all the global state needed by the interpreter.
   Really just packages more specialized structs. */
struct Vm {
  // heap state
  Heap heap;

  // scanner state
  Scanner scanner;

  // environment state
  Environment environment;
};

extern Vm vm;

// forward declarations
void   initVm(Vm* vm);
void   freeVm(Vm* vm);

#endif
