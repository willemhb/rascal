#include "environment.h"

// external API
void initEnvironment(Environment* environment) {
  environment->symbolCounter = 0;
  initSymbolTable(&environment->symbolTable);
  initNameSpace(&environment->globalNs);
  initValues(&environment->globalVals);
}

void freeEnvironment(Environment* environment) {
  freeSymbolTable(&environment->symbolTable);
  freeNameSpace(&environment->globalNs);
  freeValues(&environment->globalVals);
}

Symbol* internSymbol(Environment* environment, char* name) {
  Symbol* out = NULL;
 
  symbolTableAdd(&environment->symbolTable, name, &out);

  return out;
}

size_t defineGlobal(Environment* environment, Symbol* name, Value init) {
  size_t offset = 0;

  nameSpaceAdd(&environment->globalNs, name, &offset);

  if (offset == environment->globalVals.count) { // name was added
    if (init == NOTHING_VAL)
      init = NUL_VAL;

    writeValues(&environment->globalVals, init);
  }

  return offset;
}

Value defineSpecial(char* name, CompileFn compile) {
  Symbol* sym = getSymbol(name);
  assert(sym->special == NULL);
  sym->special = compile;
  return TAG_OBJ(sym);
}
