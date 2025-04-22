#ifndef rl_lang_h
#define rl_lang_h

#include "common.h"
#include "runtime.h"

// globals --------------------------------------------------------------------
extern Str* QuoteStr, * DefStr, * PutStr, * IfStr, * DoStr, * FnStr;

// forward declarations -------------------------------------------------------
Expr  read_exp(Port* in);
List* read_file(char* fname);
Expr  load_file(char* fname);
Expr  eval_exp(Expr x);
void  print_exp(Port* out, Expr x);
void  repl(void);

#endif
