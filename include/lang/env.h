#ifndef rl_lang_env_h
#define rl_lang_env_h

/* High level APIs for name definition and resolution. */
// headers --------------------------------------------------------------------

#include "common.h"

#include "sys/opcode.h"

// macros ---------------------------------------------------------------------

// C types --------------------------------------------------------------------

// globals --------------------------------------------------------------------

// function prototypes --------------------------------------------------------
// resolution
Ref* env_capture(Env* e, Ref* r);
Ref* env_resolve(Env* e, Sym* n, bool capture);
Ref* env_define(Env* e, Sym* n);

// toplevel environment utilities
void toplevel_env_def(Env* e, Sym* n, Expr x);
void toplevel_env_set(Env* e, Sym* n, Expr x);
void toplevel_env_refset(Env* e, int n, Expr x);
Ref* toplevel_env_find(Env* e, Sym* n);
Expr toplevel_env_ref(Env* e, int n);
Expr toplevel_env_get(Env* e, Sym* n);
void def_builtin_fun(char* name, OpCode op);

// local environment utilities
Expr upval_ref(Fun* fun, int i);
void upval_set(Fun* fun, int i, Expr x);

// initialization -------------------------------------------------------------

#endif
