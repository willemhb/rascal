#include "environment.h"


// external API
void initEnvironment(Environment* environment) {
  environment->symbolCounter = 0;
  initSymbolTable(&environment->symbolTable);
}

void freeEnvironment(Environment* environment) {
  freeSymbolTable(&environment->symbolTable);
}

Symbol* internSymbol(char* name, Environment* environment) {
  Symbol* out = NULL;

  symbolTableAdd(&environment->symbolTable, name, &out);

  return out;
}
