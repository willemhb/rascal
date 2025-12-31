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
