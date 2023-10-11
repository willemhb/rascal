#ifndef rascal_environment_h
#define rascal_environment_h

#include "common.h"
#include "object.h"

// C types
struct Environment {
  uint64_t    symbolCounter;
  SymbolTable symbolTable;
  NameSpace   globalNs;
  Values      globalVals;
};

// external API
void    initEnvironment(Environment* environment);
void    freeEnvironment(Environment* environment);

Symbol* internSymbol(Environment* environment, char* name);
size_t  defineGlobal(Environment* environment, Symbol* name, Value init);
Value   defineSpecial(char* name, CompileFn compile);

#endif
