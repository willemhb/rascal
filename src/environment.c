#include "vm.h"
#include "environment.h"

// external API
void initEnvironment(Environment* environment) {
  environment->nSymbols = 0;
  initSymbolTable(&environment->symbols);
  initNameSpace(&environment->globalNs);
  initValues(&environment->globalVals);
}

void freeEnvironment(Environment* environment) {
  freeSymbolTable(&environment->symbols);
  freeNameSpace(&environment->globalNs);
  freeValues(&environment->globalVals);
}

Symbol* internSymbol(Vm* vm, char* name) {
  Symbol* out = NULL;

  symbolTableAdd(symbols(vm), name, &out);

  return out;
}

size_t defineGlobal(Vm* vm, Symbol* name, Value init) {
  size_t offset = 0;

  nameSpaceAdd(globalNs(vm), name, &offset);

  if (offset == globalVals(vm)->count) { // name was added
    if (init == NOTHING_VAL)
      init = NUL_VAL;

    writeValues(globalVals(vm), init);
  }

  return offset;
}

bool lookupGlobal(Vm* vm, Symbol* name, Value* buf) {
  size_t offset = 0;
  bool out      = nameSpaceGet(globalNs(vm), name, &offset);

  if (out)
    *buf = NUL_VAL;

  else
    *buf = globalVals(vm)->data[offset];

  return out;
}

Value defineSpecial(char* name, CompileFn compile) {
  Symbol* sym = getSymbol(name);
  assert(sym->special == NULL);
  sym->special = compile;
  return TAG_OBJ(sym);
}
