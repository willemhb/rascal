#include "util/hashing.h"

#include "equal.h"

#include "runtime.h"
#include "collection.h"
#include "table.h"
#include "environment.h"

// external API
Value get_annot_val(Value x, Value key) {
  Value out;
  
  if (is_obj(x))
    out = get_annot_obj(as_obj(x), key);

  else {
    Value v;

    if (table_get(RlVm.n.annot, key, &v))
      out = map_get(as_map(v), key);

    else
      out = NOTHING;
  }

  return out;
}

Value get_annot_obj(void* p, Value key) {
  assert(p != NULL);

  Obj* o    = p;
  Map* m    = o->annot;
  Value out = map_get(m, key);

  return out;
}

Value set_annot_val(Value x, Value key, Value value) {
  Value out = value;
  
  if (is_obj(x))
    out = set_annot_obj(as_obj(x), key, value);

  else {
    Map* m; Value vm;

    save(4, x, key, value, NOTHING);
    
    vm = tag(&EmptyMap);
    table_add(RlVm.n.annot, key, &vm);
    m = as_map(vm);
    add_saved(3, tag(m));
    m = map_set(m, key, value);
    vm = tag(m);
    table_set(RlVm.n.annot, key, vm);
  }

  return out;
}

Value set_annot_obj(void* p, Value key, Value value) {
  assert(p != NULL);

  save(3, tag((Obj*)p), key, value);

  Obj* o   = p;
  Map* m   = o->annot;
  o->annot = map_set(m, key, value);

  return value;
}

// constructors
Symbol* new_symbol(char* name, flags_t fl) {
  static uintptr_t counter = 0;

  assert(name != NULL);

  if (*name == ':') {
    name++;
    fl |= LITERAL;
  }

  assert(*name != '\0');

  Symbol* out  =   new_obj(&SymbolType, fl, 0);
  out->name    =   duplicates(NULL, name, strlen(name));
  out->idno    = ++counter;
  out->special =   NULL;

  return out;
}

Binding* new_binding(Binding* captured, Symbol* name, NameSpace* ns, size_t offset, NsType type, Value val) {
  save(4, tag(captured), tag(name), tag(ns), val);

  Binding* out   = new_obj(&BindingType, type, 0);
  out->captured  = captured;
  out->name      = name;
  out->ns        = ns;
  out->offset    = offset;
  out->value     = val;

  return out;
}

Environment* new_envt(Environment* parent, ScopeType type) {
  save(1, tag(parent));

  Environment* out = new_obj(&EnvironmentType, type, 0);
  out->parent      = parent;
  out->globals     = RlVm.n.globals;

  if (type == FUNCTION_SCOPE) {
    save(1, tag(out));
    out->locals  = new_name_space(LOCAL_NS, NULL, NULL, NULL);
    out->upvals  = new_name_space(NONLOCAL_NS, NULL, NULL, NULL);
  } else {
    out->locals  = NULL;
    out->upvals  = NULL;
  }

  return out;
}

UpValue* new_upval(size_t offset);

void intern_symbol(SymbolTable* t, SymbolTableKv* kv, char* n, void* s, void* b) {
  (void)t;
  (void)s;

  kv->val = new_symbol(n, INTERNED);
  kv->key = kv->val->name;

  *(Symbol**)b = kv->val;
}

// convenience constructors
Symbol* symbol(char* token) {
  assert(token != NULL);
  assert(*token != '\0');

  Symbol* out  = NULL;
  
  symbol_table_intern(RlVm.n.symbols, token, NULL, &out);

  return out;
}

Symbol* gensym(char* name) {
  if (name == NULL || *name == '\0')
    name = "symbol";

  return new_symbol(name, 0);
}

// utilities
Binding*  define(Environment* envt, Symbol* name, Value init, flags_t fl) {
  Binding* b = NULL;

  save(3, tag(envt), tag(name), init);

  if (envt == NULL)
    name_space_intern(RlVm.n.globals, name, &fl, &b);

  else {
    ScopeType type = get_scope_type(envt);

    if (type == GLOBAL_SCOPE)
      name_space_intern(envt->globals, name, &fl, &b);

    else
      name_space_intern(envt->locals, name, &fl, &b);
  }

  return b;
}

Binding* capture(Environment* envt, Symbol* name) {
  if (envt == NULL || get_scope_type(envt) != FUNCTION_SCOPE)
    return lookup(envt, name);

  save(2, tag(envt), tag(name));

  Binding* cap;
  bool local;

  if (!(local=name_space_get(envt->locals, name, &cap)))
    cap = capture(envt->parent, name);

  flags_t fl = LOCAL_UPVAL;

  name_space_intern(envt->upvals, name, &fl, &cap);

  return cap;
}

Binding* lookup(Environment* envt, Symbol* name) {
  Binding* out;

  if (envt == NULL) // global lookup
    name_space_get(RlVm.n.globals, name, &out);

  else {
    ScopeType type = get_scope_type(envt);

    if (type == GLOBAL_SCOPE)
      name_space_get(envt->globals, name, &out);

    else {
      if (!name_space_get(envt->locals, name, &out)) {
        if (!name_space_get(envt->upvals, name, &out)) {
          save(2, tag(envt), tag(name));
          out = capture(envt->parent, name);

          /* TODO: remember the exact point of this. */
          if (out && get_ns_type(out->ns) != GLOBAL_NS)
            name_space_intern(envt->upvals, name, NULL, &out);
        }
      }
    }
  }

  return out;
}
