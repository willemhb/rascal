#ifndef rl_lang_eval_h
#define rl_lang_eval_h

#include "common.h"

// globals --------------------------------------------------------------------
// forward declarations -------------------------------------------------------
Expr  eval_exp(RlState* rls, Expr x);
void  repl(RlState* rls);
void  toplevel_repl(RlState* rls);

#endif
