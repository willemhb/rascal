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

  // context state
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
extern Vm vm;

// external API
// vm accessors
Heap*        heap(Vm* vm);
Obj*         heapObjects(Vm* vm);
size_t       heapUsed(Vm* vm);
size_t       heapCapacity(Vm* vm);
Objects*     heapGrays(Vm* vm);
Context*     context(Vm* vm);
bool         panicking(Vm* vm);
Environment* environment(Vm* vm);
size_t       nSymbols(Vm* vm);
SymbolTable* symbols(Vm* vm);
NameSpace*   globalNs(Vm* vm);
Values*      globalVals(Vm* vm);
Reader*      reader(Vm* vm);
FILE*        source(Vm* vm);
ReaderState  readState(Vm* vm);
TextBuffer*  readBuffer(Vm* vm);
char*        token(Vm* vm);
ReadTable*   readTable(Vm* vm);
Values*      readStack(Vm* vm);
Compiler*    compiler(Vm* vm);
Chunk*       compilerChunk(Vm* vm);
Values*      compilerStack(Vm* vm);
Interpreter* interpreter(Vm* vm);
Value*       sp(Vm* vm);
Value*       vp(Vm* vm);
Value*       ep(Vm* vm);
uint16_t*    ip(Vm* vm);
Chunk*       code(Vm* vm);

void initVm(Vm* vm);
void freeVm(Vm* vm);
void syncVm(Vm* vm);

#endif
