#ifndef rascal_environment_h
#define rascal_environment_h

#include "common.h"
#include "object.h"

// C types
typedef struct {
  uint64_t    symbolCounter;
  SymbolTable symbolTable;
  Module      toplevel;
} Environment;

// external API
void initEnvironment(Environment* environment);

#endif
