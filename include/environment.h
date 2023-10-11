#ifndef rascal_environment_h
#define rascal_environment_h

#include "common.h"
#include "object.h"

// C types
struct Environment {
  uint64_t    nSymbols;
  SymbolTable symbols;
  NameSpace   globalNs;
  Values      globalVals;
};

// external API
void    initEnvironment(Environment* environment);
void    freeEnvironment(Environment* environment);

Symbol* internSymbol(Vm* vm, char* name);
size_t  defineGlobal(Vm* vm, Symbol* name, Value init);
bool    lookupGlobal(Vm* vm, Symbol* name, Value* buf);
Value   defineSpecial(char* name, CompileFn compile);

#endif
