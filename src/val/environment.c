#include "vm/memory.h"

#include "val/environment.h"
#include "val/table.h"
#include "val/symbol.h"

/* Globals */

/* Internal APIs */
// misc
// 

// Environment
static bool is_toplevel_envt(Environment* envt) {
  return envt->parent == NULL;
}

static Binding* get_module_bind(Symbol* name, Environment* envt) {
  MutDict* scope = envt->module->bindings;
  Value    val   = mdict_get(scope, tag(name));
  
  if (val == NOTHING)
    return NULL;

  return as_bind(val);
}

static Binding* get_local_bind(Symbol* name, Environment* envt) {
  MutDict* scope = envt->locals;
  Value    val  = mdict_get(scope, tag(name));

  if (val == NOTHING)
    return NULL;

  return as_bind(val);  
}

static Binding* get_upval_bind(Symbol* name, Environment* envt) {
  MutDict* scope = envt->upvals;
  Value    val  = mdict_get(scope, tag(name));
  
  if (val == NOTHING)
    return NULL;

  return as_bind(val);
}

static Binding* intern_module_bind(Symbol* name, Environment* envt, flags_t bind_type) {
  Binding* bind;
  MutDict* scope = envt->module->bindings;
  Entry*   entry = mdict_intern(scope, tag(name));

  if (entry->val == NUL) {
    bind       = new_bind(name, MODULE_SCOPE, bind_type);
    entry->val = tag(bind);
  } else {
    bind = as_bind(entry->val);
  }

  return bind;
}

static Binding* intern_local_bind(Symbol* name, Environment* envt, flags_t bind_type) {
  Binding* bind;
  MutDict* scope = envt->locals;
  size_t   offs  = scope->cnt;
  Entry*   entry = mdict_intern(scope, tag(name));

  if (entry->val == NUL) {
    bind         = new_bind(name, LOCAL_SCOPE, bind_type);
    bind->offset = offs;
    entry->val   = tag(bind);
  } else {
    bind = as_bind(entry->val);
  }

  return bind;
}

static Binding* intern_upval_bind(Binding* captured, Environment* envt, bool local) { 
  Binding* bind;
  MutDict* scope = envt->upvals;
  size_t   offs  = scope->cnt;
  Entry*   entry = mdict_intern(scope, tag(captured->name));

  if (entry->val == NUL) {
    bind                = clone_obj(captured);
    bind->offset        = offs;
    bind->parent_offset = captured->offset;
    bind->scope_type    = UPVAL_SCOPE;
    bind->local_upval   = local;
    entry->val          = tag(bind);
  } else {
    bind = as_bind(entry->val);
  }

  return bind;
}

/* External APIs */
Binding* resolve(Symbol* name, Environment* envt, bool capture) {
  Binding* out;

  if (is_toplevel_envt(envt))
    out = get_module_bind(name, envt);

  else {
    out = get_local_bind(name, envt);

    if (out != NULL) {
      if (capture) // called by a child environment searching for an upvalue
        out = intern_upval_bind(out, envt, true);

    } else {
      out = get_upval_bind(name, envt);

      if (out == NULL) {
        out = resolve(name, envt->parent, true);

        if (out != NULL && out->scope_type == UPVAL_SCOPE)
          out = intern_upval_bind(out, envt, false);
      }
    }
  }

  return out;
}

Binding* define(Symbol* name, Environment* envt, flags_t bind_type) {
  if (is_toplevel_envt(envt))
    return intern_module_bind(name, envt, bind_type);

  return intern_local_bind(name, envt, bind_type);
}
