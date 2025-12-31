#ifndef rl_lang_env_h
#define rl_lang_env_h

#include "val/env.h"

/* main logic for handling lookups and assignments. */
// Prototypes -----------------------------------------------------------------
// environment map methods
void intern_in_env(RlState* rls, EMap* m, EMapKV* kv, Sym* k, hash_t h);

// main environment operations
Ref* env_capture(RlState* rls, Env* e, Ref* r);
Ref* env_resolve(RlState* rls, Env* e, Sym* n, bool capture);
Ref* env_define(RlState* rls, Env* e, Sym* n, bool m, bool f, bool *a);

// toplevel environment helpers
void toplevel_env_def(RlState* rls, Env* e, Sym* n, Expr x, bool m, bool f);
void toplevel_env_set(RlState* rls, Env* e, Sym* n, Expr x);
Ref* toplevel_env_find(RlState* rls, Env* e, Sym* n);
Expr toplevel_env_get(RlState* rls, Env* e, Sym* n);

// helpers for defining builtins
Fun* def_builtin_fun(RlState* rls, char* name, int arity, bool va, OpCode op);

#endif
