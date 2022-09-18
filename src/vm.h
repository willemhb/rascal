#ifndef rascal_vm_h
#define rascal_vm_h

#include "table.h"
#include "reader.h"
#include "function.h"

// parameters -----------------------------------------------------------------
// C types --------------------------------------------------------------------
typedef struct ArrayList Stack;

typedef struct VM
{
  // interpreter state --------------------------------------------------------
  Stack       *stack;
  Closure     *closure;
  Instruction *ip;

  // environment and symbol table state ---------------------------------------
  Map         *globals;
  Table       *symbols;

  // reader state -------------------------------------------------------------
  Scanner     *scanner;
  Parser      *parser;

  // memory management state --------------------------------------------------
  UpValue     *openUpvalues;
  Obj         *objects;
  Size         bytesAllocated;
  Size         nextGC;
  Stack       *grayStack;
} VM;

// globals --------------------------------------------------------------------
extern VM vm;

#endif
