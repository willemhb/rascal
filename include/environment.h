#ifndef rascal_environment_h
#define rascal_environment_h

#include "common.h"
#include "object.h"

// C types
struct Environment {
  uint64_t    symbolCounter;
  SymbolTable symbolTable;
};

// external API
void    initEnvironment(Environment* environment);
void    freeEnvironment(Environment* environment);
Symbol* internSymbol(char* name, Environment* environment);

#endif
