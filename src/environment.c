#include "util/hashing.h"

#include "equal.h"

#include "vm.h"
#include "memory.h"
#include "collection.h"
#include "table.h"
#include "environment.h"

// external API
void initEnvt(Vm* vm) {
  vm->envt.symbols = newSymbolTable(0, NULL, NULL, NULL);
  vm->envt.globals = newNameSpace(GLOBAL_NS, NULL, NULL, NULL);
  vm->envt.annot   = newTable(0, NULL, hashWord, same);
  vm->envt.used    = newTable(0, NULL, NULL, NULL);
}

void freeEnvt(Vm* vm) {
  freeSymbolTable(vm->envt.symbols);
  freeTable(vm->envt.globals);
  freeTable(vm->envt.annot);
  freeTable(vm->envt.used);
}

Value getAnnotVal(Value x, Value key) {
  Value out;
  
  if (IS_OBJ(x))
    out = getAnnotObj(AS(Obj,x), key);

  else {
    Value v;

    if (tableGet(RlVm.envt.annot, key, &v))
      out = mapGet(AS(Map, v), key);

    else
      out = NOTHING;
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
  Value out = value;
  
  if (IS_OBJ(x))
    out = setAnnotObj(AS(Obj, x), key, value);

  else {
    Map* m; Value vm;
    
    save(3, x, key, value);
    vm = tag(&EmptyMap);
    tableAdd(RlVm.envt.annot, key, &vm);
    save(1, vm);
    m = AS(Map, vm);
    m = mapSet(m, key, value);
    vm = tag(m);
    save(1, vm);
    tableSet(RlVm.envt.annot, key, vm);
    unsave(5);
  }

  return out;
}

Value setAnnotObj(void* p, Value key, Value value) {
  assert(p != NULL);

  save(3, tagObj(p), key, value);

  Obj* o   = p;
  Map* m   = o->annot;
  o->annot = mapSet(m, key, value);

  unsave(3);

  return value;
}

// constructors
Symbol* newSymbol(char* name, flags_t fl) {
  static uintptr_t counter = 0;

  assert(name != NULL);

  if (*name == ':') {
    name++;
    fl |= LITERAL;
  }

  assert(*name != '\0');

  Symbol* out  =   newObj(&SymbolType, fl, 0);
  out->name    =   duplicates(NULL, name, strlen(name));
  out->idno    = ++counter;
  out->special =   NULL;

  return out;
}

Binding* newBinding(Binding* captured, Symbol* name, NameSpace* ns, size_t offset, NsType type, Value val) {
  size_t nSaved = save(4, tag(captured), tag(name), tag(ns), val);

  Binding* out   = newObj(&BindingType, type, 0);
  out->captured  = captured;
  out->name      = name;
  out->ns        = ns;
  out->offset    = offset;
  out->value     = val;

  unsave(nSaved);

  return out;
}

Environment* newEnvironment(Environment* parent, ScopeType type) {
  size_t nSaved = save(1, tag(parent));

  Environment* out = newObj(&EnvironmentType, type, 0);
  out->parent      = parent;

  out->globals     = RlVm.envt.globals;

  if (type == PRIVATE_SCOPE) {
    nSaved += save(1, tag(out));
    out->private = newNameSpace(PRIVATE_NS, NULL, NULL, NULL);
    out->locals  = NULL;
    out->upvals  = NULL;
  } else if (type == FUNCTION_SCOPE) {
    nSaved += save(1, tag(out));
    out->private = parent->private;
    out->locals  = newNameSpace(LOCAL_NS, NULL, NULL, NULL);
    out->upvals  = newNameSpace(NONLOCAL_NS, NULL, NULL, NULL);
  } else {
    out->private = NULL;
    out->locals  = NULL;
    out->upvals  = NULL;
  }
  
  unsave(nSaved);

  return out;
}

UpValue* newUpValue(size_t offset);

void internSymbol(SymbolTable* t, SymbolTableKv* kv, char* n, void* s, void* b) {
  (void)t;
  (void)s;

  kv->val = newSymbol(n, INTERNED);
  kv->key = kv->val->name;

  *(Symbol**)b = kv->val;
}

// convenience constructors
Symbol* symbol(char* token) {
  assert(token != NULL);
  assert(*token != '\0');

  Symbol* out  = NULL;

  symbolTableIntern(RlVm.envt.symbols, token, NULL, &out);

  return out;
}

Symbol* gensym(char* name) {
  if (name == NULL || *name == '\0')
    name = "symbol";

  return newSymbol(name, 0);
}

// utilities
Binding*  define(Environment* envt, void* p, Value init, bool internal) {
  size_t nSaved = save(2, tag(envt), init);
  
  Symbol* name = internal ? symbol(p) : p;
  Binding* b = NULL;

  nSaved += save(1, tag(name));

  if (envt == NULL)
    nameSpaceIntern(RlVm.envt.globals, name, &init, &b);

  else {
    ScopeType type = getScopeType(envt);

    if (type == GLOBAL_SCOPE)
      nameSpaceIntern(envt->globals, name, &init, &b);

    else if (type == PRIVATE_SCOPE)
      nameSpaceIntern(envt->private, name, &init, &b);

    else
      nameSpaceIntern(envt->locals, name, &init, &b);
  }

  unsave(nSaved);

  return b;
}

Binding* capture(Environment* envt, Symbol* name) {
  if (envt == NULL || getScopeType(envt) != FUNCTION_SCOPE)
    return lookup(envt, name);

  Binding* cap;
  size_t ns = save(2, tag(envt), tag(name));
  bool local;

  if (!(local=nameSpaceGet(envt->locals, name, &cap)))
    cap = capture(envt->parent, name);

  nameSpaceIntern(envt->upvals, name, NULL, &cap);
  setFl(cap, LOCAL_UPVAL);
  unsave(ns);

  return cap;
}

Binding* lookup(Environment* envt, Symbol* name) {
  Binding* out;

  if (envt == NULL) // global lookup
    nameSpaceGet(RlVm.envt.globals, name, &out);

  else {
    ScopeType type = getScopeType(envt);

    if (type == GLOBAL_SCOPE)
      nameSpaceGet(envt->globals, name, &out);

    else if (type == PRIVATE_SCOPE) {
      if (!nameSpaceGet(envt->private, name, &out))
        nameSpaceGet(envt->globals, name, &out);
    } else {
      if (!nameSpaceGet(envt->locals, name, &out)) {
        if (!nameSpaceGet(envt->upvals, name, &out)) {
          size_t ns = save(2, tag(envt), tag(name));
          out = capture(envt->parent, name);

          if (out && getNsType(out->ns) > PRIVATE_NS)
            nameSpaceIntern(envt->upvals, name, NULL, &out);

          unsave(ns);
        }
      }
    }
  }

  return out;
}
