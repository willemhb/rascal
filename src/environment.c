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

  symbolTableAdd(&vm->environment.symbols, name, &out);

  return out;
}

size_t defineGlobal(Vm* vm, Symbol* name, Value init) {
  size_t offset = 0;
  Environment* env = &vm->environment;
  NameSpace* ns = &env->globalNs;
  Values* vals = &env->globalVals;

  nameSpaceAdd(ns, name, &offset);

  if (offset == vals->count) { // name was added
    if (init == NOTHING)
      init = NUL;

    writeValues(vals, init);
  }

  return offset;
}

bool lookupGlobal(Vm* vm, Symbol* name, Value* buf) {
  size_t offset = 0;
  Environment* env = &vm->environment;
  NameSpace* ns = &env->globalNs;
  Values* vals = &env->globalVals;
  bool out = nameSpaceGet(ns, name, &offset);

  if (!out)
    *buf = NUL;

  else
    *buf = vals->data[offset];

  return out;
}

Value defineSpecial(char* name, CompileFn compile) {
  Symbol* sym = getSymbol(name);
  assert(sym->special == NULL);
  sym->special = compile;
  return TAG_OBJ(sym);
}
