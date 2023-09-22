#ifndef rascal_vm_h
#define rascal_vm_h

#include "common.h"
#include "value.h"
#include "object.h"
#include "lexer.h"
#include "parser.h"
#include "compiler.h"

#define N_STACK 65536
#define N_FRAME 8192

// activation record
typedef struct {
  Closure*  closure;
  uint16_t* ip;
  Value*    slots;
} CallFrame;

// global environment
#include "tpl/declare.h"

TABLE_TYPE(GlobalEnv, globalEnv, Symbol*, Value);

// global interpreter state
typedef struct {
  // heap state
  Obj*    objects;
  size_t  heapUsed;
  size_t  heapCapacity;
  Objects grays;

  // interpreter state
  Value*     sp;
  CallFrame* fp;
  UpValue*   openUpvalues;

  // globals/symbol table
  uint64_t    symbolCounter;
  SymbolTable symbolTable;
  GlobalEnv   globalEnv;

  // scanner state
  Scanner scanner;

  // parser state
  Parser parser;

  // compiler state
  Compiler compiler;
} Vm;

extern Value theStack[N_STACK];
extern CallFrame theFrames[N_FRAME];
extern Vm vm;

// forward declarations
void  initVm(Vm* vm);
void  freeVm(Vm* vm);
void  push(Value value);
Value pop(void);
Value peek(int n);

#endif
