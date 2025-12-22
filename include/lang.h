#ifndef rl_lang_h
#define rl_lang_h

#include "common.h"
#include "runtime.h"

// globals --------------------------------------------------------------------
// forward declarations -------------------------------------------------------
Expr  read_exp(RlState* rls, Port* in);
List* read_file(RlState* rls, char* fname);
List* read_file_s(RlState* rls, char* fname);
Expr  load_file(RlState* rls, char* fname);
Expr  eval_exp(RlState* rls, Expr x);
void  print_exp(Port* out, Expr x);
void  repl(RlState* rls);

#endif
