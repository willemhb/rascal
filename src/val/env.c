#include "val/env.h"
#include "val/port.h"
#include "vm.h"
#include "lang.h"

// forward declarations
void print_ref(Port* ios, Expr x);
void print_env(Port* ios, Expr x);
void trace_ref(RlState* rls, void* ptr);
void trace_upval(RlState* rls, void* ptr);
void trace_env(RlState* rls, void* ptr);
void free_env(RlState* rls, void* ptr);

// Type objects
Type RefType = {
  .heap     = NULL,
  .type     = &TypeType,
  .bfields  = FL_GRAY,
  .tag      = EXP_REF,
  .obsize   = sizeof(Ref),
  .print_fn = print_ref,
  .trace_fn = trace_ref
};

Type UpValType = {
  .heap     = NULL,
  .type     = &TypeType,
  .bfields  = FL_GRAY,
  .tag      = EXP_UPV,
  .obsize   = sizeof(UpVal),
  .trace_fn = trace_upval
};

Type EnvType = {
  .heap     = NULL,
  .type     = &TypeType,
  .bfields  = FL_GRAY,
  .tag      = EXP_ENV,
  .obsize   = sizeof(Env),
  .print_fn = print_env,
  .trace_fn = trace_env,
  .free_fn  = free_env
};

// reference API
Ref* mk_ref(RlState* rls, Sym* n, int o) {
  Ref* ref  = mk_obj(rls, &RefType, 0);
  ref->name = n;
  ref->ref_type = REF_UNDEF; // filled in by env_put, env_resolve, &c
  ref->offset = o;
  ref->macro = false; // set elsewhere
  ref->final = false; // set elsewhere
  ref->val = NONE;

  return ref;
}

void print_ref(Port* ios, Expr x) {
  Ref* r = as_ref(x);

  pprintf(ios, "#'%s", r->name->val->val);
}

void trace_ref(RlState* rls, void* ptr) {
  Ref* r = ptr;

  mark_obj(rls, r->captures);
  mark_obj(rls, r->name);
  mark_expr(rls, r->val);
}

// upval API
UpVal* mk_upval(RlState* rls, UpVal* next, Expr* loc) {
  // only open upvalues can be created
  UpVal* upv  = mk_obj(rls, &UpValType, 0);
  upv->next   = next;
  upv->closed = false;
  upv->loc    = loc;

  return upv;
}

Expr* deref(UpVal* upv) {
  return upv->closed ? &upv->val : upv->loc;
}

void trace_upval(RlState* rls, void* ptr) {
  UpVal* upv = ptr;

  if ( upv->closed )
    mark_expr(rls, upv->val);
}

// environment API
void intern_in_env(RlState* rls, EMap* m, EMapKV* kv, Sym* k, hash_t h) {
  (void)h;

  kv->key = k;
  kv->val = mk_ref(rls, k, m->count-1);
}

Env* mk_env(RlState* rls, Env* parent) {
  Env* env = mk_obj(rls, &EnvType, 0);

  env->parent = parent;
  env->ncap = 0;
  env->etype = parent == NULL ? REF_GLOBAL : REF_LOCAL;
  init_emap(rls, &env->vars);
  init_emap(rls, &env->upvs);

  return env;
}

Env* mk_env_s(RlState* rls, Env* parent) {
  Env* out = mk_env(rls, parent);
  stack_push(rls, tag_obj(out));

  return out;
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

void toplevel_env_refset(RlState* rls, Env* e, int n, Expr x) {
  (void)rls;

  assert(is_global_env(e));
  assert(n >= 0 && n < e->vals.count);
  Ref* r = e->vals.data[n];

  // don't overwrite final bindings
  if ( r->final && r->val != NONE )
    eval_error(rls, "cannot assign to initialized final ref %s", r->name->val->val);

  r->val = x;
}

Ref* toplevel_env_find(RlState* rls, Env* e, Sym* n) {
  assert(is_global_env(e));

  Ref* ref = NULL;

  emap_get(rls, &e->vars, n, &ref);

  return ref;
}

Expr toplevel_env_ref(RlState* rls, Env* e, int n) {
  (void)rls;

  assert(is_global_env(e));
  assert(n >= 0 && n < e->vals.count);

  Ref* r = e->vals.data[n];

  return r->val;
}

Expr toplevel_env_get(RlState* rls, Env* e, Sym* n) {
  assert(is_global_env(e));
  Expr x = NONE;
  Ref* ref = toplevel_env_find(rls, e, n);

  if ( ref !=  NULL )
    x = ref->val;

  return x;
}

static void trace_emap(RlState* rls, EMap* m) {
  for ( int i=0, j=0; i < m->maxc && j < m->count; i++ ) {
    EMapKV* kv = &m->kvs[i];

    if ( kv->key != NULL ) {
      j++;
      mark_obj(rls, kv->key);
      mark_obj(rls, kv->val);
    }
  }
}

void print_env(Port* ios, Expr x) {
  Env* e = as_env(x);

  if ( is_global_env(e) )
    pprintf(ios, "<global env>");

  else
    pprintf(ios, "<local env>");
}

void trace_env(RlState* rls, void* ptr) {
  Env* e = ptr;

  mark_obj(rls, e->parent);

  trace_emap(rls, &e->vars);

  if ( is_local_env(e) )
    trace_emap(rls, &e->upvs);

  else
    trace_objs(rls, &e->vals);
}

void free_env(RlState* rls, void* ptr) {
  Env* e = ptr;

  free_emap(rls, &e->vars);

  if ( is_local_env(e) )
    free_emap(rls, &e->upvs);

  else
    free_objs(rls, &e->vals);
}
