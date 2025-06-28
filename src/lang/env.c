/* High level APIs for name definition and resolution. */
// headers --------------------------------------------------------------------
#include "data/expr.h"
#include "data/types/env.h"
#include "data/types/ref.h"
#include "data/types/sym.h"

#include "lang/env.h"

// macros ---------------------------------------------------------------------

// C types --------------------------------------------------------------------

// globals --------------------------------------------------------------------

// function prototypes --------------------------------------------------------

// function implementations ---------------------------------------------------
// internal -------------------------------------------------------------------

// external -------------------------------------------------------------------
void intern_in_env(EMap* m, EMapKV* kv, Sym* k, hash_t h) {
  (void)h;

  kv->key = k;
  kv->val = mk_ref(k, m->count-1);
}

Ref* env_capture(Env* e, Ref* r) {
  assert(is_local_env(e));

  Ref* c = emap_intern(&e->upvs, r->name, intern_in_env);

  if ( c->ref_type == REF_UNDEF ) {
    c->ref_type = r->ref_type == REF_LOCAL ? REF_LOCAL_UPVAL : REF_CAPTURED_UPVAL;
    c->captures = r;
  }

  return c;
}

Ref* env_resolve(Env* e, Sym* n, bool capture) {
  Ref* r = NULL;

  if ( is_global_env(e) )
    emap_get(&e->vars, n, &r);

  else {
    bool found;

    found = emap_get(&e->vars, n, &r); // check locals first

    if ( found ) {
      if ( capture ) {         // resolved from enclosed context
        r = env_capture(e, r);
        e->ncap++;             // so we know to emit the capture instruction
      }

    } else {
      // check already captured upvalues
      found = emap_get(&e->upvs, n, &r);

      if ( !found ) {
        r = env_resolve(e->parent, n, true);

        if ( r != NULL && r->ref_type != REF_GLOBAL )
          r = env_capture(e, r);
      }
    }
  }

  return r;
}

Ref* env_define(Env* e, Sym* n) {
  Ref* ref = emap_intern(&e->vars, n, intern_in_env);

  if ( ref->ref_type == REF_UNDEF ) {
    if ( is_local_env(e) )
      ref->ref_type = REF_LOCAL;

    else {
      ref->ref_type = REF_GLOBAL;
      exprs_push(&e->vals, NONE); // reserve space for value
    }
  }

  return ref;
}

// helpers for working with the global environment
void toplevel_env_def(Env* e, Sym* n, Expr x) {
  assert(is_global_env(e));
  assert(!is_keyword(n));

  Ref* r = env_define(e, n);

  e->vals.vals[r->offset] = x;
}

void toplevel_env_set(Env* e, Sym* n, Expr x) {
  assert(is_global_env(e));
  assert(!is_keyword(n));

  Ref* r = toplevel_env_find(e, n);

  assert(r != NULL);

  e->vals.vals[r->offset] = x;
}

void toplevel_env_refset(Env* e, int n, Expr x) {
  assert(is_global_env(e));
  assert(n >= 0 && n < e->vals.count);
  e->vals.vals[n] = x;
}

Ref* toplevel_env_find(Env* e, Sym* n) {
  assert(is_global_env(e));

  Ref* ref = NULL;

  emap_get(&e->vars, n, &ref);

  return ref;
}

Expr toplevel_env_ref(Env* e, int n) {
  assert(is_global_env(e));
  assert(n >= 0 && n < e->vals.count);

  return e->vals.vals[n];
}

Expr toplevel_env_get(Env* e, Sym* n) {
  assert(is_global_env(e));
  Expr x = NONE;
  Ref* ref = toplevel_env_find(e, n);

  if ( ref !=  NULL )
    x = e->vals.vals[ref->offset];

  return x;
}

void def_builtin_fun(char* name, OpCode op) {
  Sym* sym = mk_sym(name); preserve(1, tag_obj(sym));
  Fun* fun = mk_builtin_fun(sym, op);

  toplevel_env_def(&Globals, sym, tag_obj(fun));
}


Expr upval_ref(Fun* fun, int i) {
  assert(i >= 0 && i < fun->upvs.count);
  UpVal* upv = fun->upvs.vals[i];

  return *deref(upv);
}

void upval_set(Fun* fun, int i, Expr x) {
  assert(i >= 0 && i < fun->upvs.count);
  UpVal* upv = fun->upvs.vals[i];

  *deref(upv) = x;
}

// initialization -------------------------------------------------------------
