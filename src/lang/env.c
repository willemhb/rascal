#include <stdarg.h>

#include "lang/env.h"
#include "lang/dispatch.h"
#include "val.h"
#include "vm.h"

// C types --------------------------------------------------------------------
// Globals --------------------------------------------------------------------
// Prototypes -----------------------------------------------------------------
// Implementations ------------------------------------------------------------
void intern_in_env(RlState* rls, EMap* m, EMapKV* kv, Sym* k, hash_t h) {
  (void)h;

  kv->key = k;
  kv->val = mk_ref(rls, k, m->count-1);
}

Ref* env_capture(RlState* rls, Env* e, Ref* r) {
  assert(is_local_env(e));

  Ref* c = emap_intern(rls, &e->upvs, r->name, intern_in_env);

  if ( c->ref_type == REF_UNDEF ) {
    c->ref_type = r->ref_type == REF_LOCAL ? REF_LOCAL_UPVAL : REF_CAPTURED_UPVAL;
    c->captures = r;
  }

  return c;
}

Ref* env_resolve(RlState* rls, Env* e, Sym* n, bool capture) {
  Ref* r = NULL;

  if ( is_global_env(e) )
    emap_get(rls, &e->vars, n, &r);

  else {
    bool found;

    found = emap_get(rls, &e->vars, n, &r); // check locals first

    if ( found ) {
      if ( capture ) {         // resolved from enclosed context
        r = env_capture(rls, e, r);
        e->ncap++;             // so we know to emit the capture instruction
      }
    } else {
      // check already captured upvalues
      found = emap_get(rls, &e->upvs, n, &r);

      if ( !found ) {
        r = env_resolve(rls, e->parent, n, true);

        if ( r != NULL && r->ref_type != REF_GLOBAL )
          r = env_capture(rls, e, r);
      }
    }
  }

  return r;
}

Ref* env_define(RlState* rls, Env* e, Sym* n, bool m, bool f, bool* a) {
  assert(is_global_env(e) || !m);
  Ref* ref = emap_intern(rls, &e->vars, n, intern_in_env);
  bool new = ref->ref_type == REF_UNDEF;

  if ( a )
    *a = new;

  if ( new ) { // signals freshly created ref
    ref->macro = m;
    ref->final = f;

    if ( is_local_env(e) )
      ref->ref_type = REF_LOCAL;

    else {
      ref->ref_type = REF_GLOBAL;
      objs_push(rls, &e->vals, ref); // reserve space for value
    }
  }

  return ref;
}

// helpers for working with the global environment
void toplevel_env_def(RlState* rls, Env* e, Sym* n, Expr x, bool m, bool f) {
  assert(is_global_env(e));
  assert(!is_keyword(n));

  Ref* r = env_define(rls, e, n, m, f, NULL);
  r->val = x;
}

void toplevel_env_set(RlState* rls, Env* e, Sym* n, Expr x) {
  assert(is_global_env(e));
  assert(!is_keyword(n));

  Ref* r = toplevel_env_find(rls, e, n);
  assert(r != NULL);

  // don't overwrite final bindings
  if ( r->final && r->val != NONE )
    eval_error(rls, "cannot assign to initialized final ref %s", n->val->val);

  r->val = x;
}

Ref* toplevel_env_find(RlState* rls, Env* e, Sym* n) {
  assert(is_global_env(e));

  Ref* ref = NULL;

  emap_get(rls, &e->vars, n, &ref);

  return ref;
}

Expr toplevel_env_get(RlState* rls, Env* e, Sym* n) {
  assert(is_global_env(e));
  Expr x = NONE;
  Ref* ref = toplevel_env_find(rls, e, n);

  if ( ref !=  NULL )
    x = ref->val;

  return x;
}

// helpers for defining builtins
void add_builtin_method(RlState* rls, Fun* fun, OpCode op, bool va, int arity, ...) {
  StackRef top = rls->s_top;
  va_list types;
  va_start(types, arity);

  for ( int i=0; i < arity; i++ ) {
    Type* t = va_arg(types, Type*);
    stack_push(rls, tag_obj(t));
  }

  va_end(types);
  Tuple* sig = mk_tuple_s(rls, arity);
  Method* m = mk_builtin_method_s(rls, fun, va, arity, sig, op);
  fun_add_method(rls, fun, m);
  rls->s_top = top;
}

Fun* def_builtin_fun(RlState* rls, char* name, OpCode op, bool va, int arity, ...) {
  StackRef top = rls->s_top;
  bool added = false;

  Sym* n = mk_sym_s(rls, name);
  Ref* r = env_define(rls, rls->vm->globals, n, false, true, &added);
  Fun* f;

  if ( added ) {
    f = mk_fun_s(rls, n, false, true);
    r->val = tag_obj(f);
  } else {
    f = as_fun(r->val);
  }

  // push signature types
  va_list types;
  va_start(types, arity);

  for ( int i=0; i < arity; i++ ) {
    Type* t = va_arg(types, Type*);
    stack_push(rls, tag_obj(t));
  }

  va_end(types);
  Tuple* s = mk_tuple_s(rls, arity); // build signature
  Method* m = mk_builtin_method_s(rls, f, va, arity, s, op); // create method
  fun_add_method(rls, f, m); // add method  
  rls->s_top = top; // reset stack

  return f;
}
