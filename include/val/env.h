#ifndef rl_env_h
#define rl_env_h

#include "val/val.h"
#include "val/sym.h"

// variable reference metadata
typedef enum {
  REF_UNDEF,
  REF_GLOBAL,
  REF_LOCAL,
  REF_LOCAL_UPVAL,
  REF_CAPTURED_UPVAL
} RefCode;

struct Ref {
  HEAD;
  Ref* captures;
  Sym* name;
  RefCode ref_type;
  int offset;
  bool macro;   // true if this ref points to a macro
  bool final;   // true if this ref can't be reassigned (initial value is always NONE)
  Expr val;     // only used at Global scope
};

// special indirecting containers for closure variables
struct UpVal {
  HEAD;

  UpVal* next;
  bool   closed;

  union {
    Expr  val;
    Expr* loc;
  };
};

// standard representation for compile-time and run-time environments
struct Env {
  HEAD;

  Env* parent;
  RefCode etype;
  int ncap; // number of captured *local* upvalues

  EMap vars; // personal namespace

  union {
    EMap upvs;
    Objs vals;
  };
};

// ref API
Ref* mk_ref(RlState* rls, Sym* n, int o);

// upval APIs
UpVal* mk_upval(RlState* rls, UpVal* next, Expr* loc);
Expr*  deref(UpVal* upv);

// environment API
Env*   mk_env(RlState* rls, Env* parent);
Env*   mk_env_s(RlState* rls, Env* parent);
Ref*   env_capture(RlState* rls, Env* e, Ref* r);
Ref*   env_resolve(RlState* rls, Env* e, Sym* n, bool capture);
Ref*   env_define(RlState* rls, Env* e, Sym* n, bool m, bool f, bool *a);
void   toplevel_env_def(RlState* rls, Env* e, Sym* n, Expr x, bool m, bool f);
void   toplevel_env_set(RlState* rls, Env* e, Sym* n, Expr x);
void   toplevel_env_refset(RlState* rls, Env* e, int n, Expr x);
Ref*   toplevel_env_find(RlState* rls, Env* e, Sym* n);
Expr   toplevel_env_ref(RlState* rls, Env* e, int n);
Expr   toplevel_env_get(RlState* rls, Env* e, Sym* n);

// convenience macros & accessors
#define as_ref(x)        ((Ref*)as_obj(x))
#define as_env(x)        ((Env*)as_obj(x))
#define as_env_s(rls, x) ((Env*)as_obj_s(rls, &EnvType, x))

#define is_local_env(e)    ((e)->parent != NULL)
#define is_global_env(e)   ((e)->parent == NULL)
#define env_type(e)        ((e)->etype)
#define env_upval_maxc(e)  ((e)->upvs.maxc)
#define env_upval_refs(e)  ((e)->upvs.kvs)

static inline int env_size(Env* e) {
  if ( is_global_env(e) )
    return e->vars.count;

  return e->vars.count + e->upvs.count;
}

static inline int env_upvc(Env* e) {
  if ( is_global_env(e) )
    return 0;

  return e->upvs.count;
}

#endif
