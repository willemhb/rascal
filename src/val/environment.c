#include "vm/memory.h"

#include "val/environment.h"
#include "val/table.h"
#include "val/symbol.h"

/* External APIs */
Binding* new_bind(Symbol* name, flags_t scope_type, flags_t bind_type) {
  save(1, tag(name));

  Binding* out = new_obj(&BindingType, 0, 0);

  // copy metadata from symbol
  out->obj.meta    = name->obj.meta;
  out->name        = name;
  out->scope_type  = scope_type;
  out->bind_type   = bind_type;
  out->initialized = false;
  out->local_upval = false;

  return out;
}

/* Scope API */
static bool cmp_syms(Symbol* sx, Symbol* sy) {
  return sx->ns == sy->ns && sx->name == sy->name && sx->idno == sy->idno;
}

void init_scope(Scope* scope) {
  scope->data = NULL;
  scope->map  = NULL;
  scope->cnt  = 0;
  scope->mcap = 0;
  scope->ecap = 0;
}

void free_scope(Scope* scope) {
  deallocate(scope->data, 0, false);
  deallocate(scope->map, 0, false);
  init_scope(scope);
}

void resize_scope_map(Scope* scope, size_t new_cnt) {
  
}

void resize_scope_entries(Scope* scope, size_t new_cnt) {
  
}


short* find_scope_entry(ScopeEntry* entries, short* map, size_t mc, Symbol* name) {
  hash_t h;
  size_t i, m;

  h = name->obj.hash;
  m = mc - 1;
  i = h & m;

  for (;;) {
    short* mloc = &map[i];

    if (*mloc == -1)
      return mloc;

    ScopeEntry* entry = &entries[*mloc];

    if (cmp_syms(entry->name, name))
      return mloc;

    i = (i + 1) & m;
  }
}

ScopeEntry* add_scope_binding(Scope* scope, Symbol* name, flags_t bind_type) {
  ScopeEntry* entry;
  flags_t scope_type;

  save(2, tag(scope), tag(name));

  resize_scope_entries(scope, scope->cnt+1);

  scope_type  = scope->scope_type;
  entry       = &scope->data[scope->cnt];
  entry->name = name;
  entry->bind = new_bind(name, scope_type, bind_type);

  scope->cnt++;

  return entry;
}

ScopeEntry* share_scope_binding(Scope* scope, Symbol* name, Binding* bind) {
  ScopeEntry* entry;

  resize_scope_entries(scope, scope->cnt+1);

  entry       = &scope->data[scope->cnt];
  entry->name = name;
  entry->bind = bind;
  scope->cnt++;

  return entry;
}

ScopeEntry* copy_scope_binding(Scope* scope, Symbol* name, Binding* bind) {
  save(3, tag(scope), tag(name), tag(bind));
  
  return share_scope_binding(scope, name, clone_obj(bind));
}

Scope* new_scope(flags_t scope_type) {
  Scope* out = new_obj(&ScopeType, 0, 0);

  init_scope(out);
  out->scope_type = scope_type;

  return out;
}

/* Environment API */
Environment* new_envt(Environment* parent) {
  Environment* out;

  if (parent != NULL) {
    save(2, tag(parent), NOTHING);

    out = new_obj(&EnvironmentType, 0, 0);

    add_saved(tag(out), 1);

    out->parent   = parent;
    out->module   = parent->module;
    out->locals   = new_scope(LOCAL_SCOPE);
    out->upvals   = new_scope(UPVAL_SCOPE);
  } else {
    out = new_obj(&EnvironmentType, 0, 0);

    save(1, tag(out));

    out->parent = NULL;
    out->module = new_scope(MODULE_SCOPE);
    out->locals = NULL;
    out->upvals = NULL;
  }

  return out;
}


Binding* resolve(Symbol* name, Environment* envt, bool capture, flags_t* scope_type, size_t* offset) {
  Binding* out = NULL;
  
  if (envt != NULL) {
  Scope* scope;
  short* location;

    if (envt->locals != NULL) {
      scope = envt->locals;
      location = find_scope_entry(scope->data, scope->map, scope->mcap, name);

      if (*location != -1)
        out = scope->data[*location].bind;

      else { // check upvalues
        scope    = envt->upvals;
        location 
      }
    }

    if (out != NULL) {
      if ()
    }
  }

  return out;
}

Binding* define(Symbol* name, Environment* envt, flags_t bind_type, flags_t* scope_type, size_t* offset) {
  
}
