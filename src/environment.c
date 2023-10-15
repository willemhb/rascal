#include "vm.h"
#include "environment.h"

// external API
void initEnvt(Vm* vm) {
  vm->toplevel.nSymbols = 0;
  
  initSymbolCache(&vm->toplevel.symbols);
  initAnnotations(&vm->toplevel.annot);
  initNsMap(&vm->toplevel.globals);
  initLoadCache(&vm->toplevel.used);
}

void freeEnvt(Vm* vm) {
  freeLoadCache(&vm->toplevel.used);
  freeNsMap(&vm->toplevel.globals);
  freeAnnotations(&vm->toplevel.annot);
  freeSymbolCache(&vm->toplevel.symbols);
}

Value getAnnotVal(Value x, Value key) {
  Value out;
  
  if (IS_OBJ(x))
    out = getAnnotObj(AS_OBJ(x), key);

  else {
    Map* m;

    annotationsGet(&RlVm.toplevel.annot, key, &m);
    out = m ? mapGet(m, key) : NOTHING;
  }

  return out;
}

Value getAnnotObj(void* p, Value key) {
  assert(p != NULL);

  Obj* o    = p;
  Map* m    = o->annot;
  Value out = mapGet(m, key);

  return out;
}

Value setAnnotVal(Value x, Value key, Value value) {
  Value out;
  
  if (IS_OBJ(x))
    out = setAnnotObj(AS_OBJ(x), key, value);

  else {
    Map* m;

    save(3, x, key, value);        
    annotationsGet(&RlVm.toplevel.annot, key, &m);
    
    if (m == NULL)
      m = &emptyMap;
    
    m = mapSet(m, key, value);
    out = value;
    annotationsSet(&RlVm.toplevel.annot, key, m);
    unsave(3);
  }

  return out;
}

Value setAnnotObj(void* p, Value key, Value value) {
  assert(p != NULL);

  Obj* o   = p;
  Map* m   = o->annot;
  Map* u   = mapSet(m, key, value);
  o->annot = u;

  return value;
}

Symbol* internSymbol(Vm* vm, char* name) {
  Symbol* out = NULL;

  symbolCacheAdd(&vm->toplevel.symbols, name, &out);

  return out;
}

size_t defineGlobal(Vm* vm, Symbol* name, Value init, int flags) {
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
  return tag(sym);
}
