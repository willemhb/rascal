#ifndef rl_lang_h
#define rl_lang_h

#include "common.h"
#include "runtime.h"

// globals --------------------------------------------------------------------
extern Str* QuoteStr, * DefStr, * PutStr, * IfStr, * DoStr, * FnStr,
  * CatchStr, * ThrowStr;

// forward declarations -------------------------------------------------------
Expr  read_exp(Port* in);
Expr  eval_exp(Expr x);
void  print_exp(Port* out, Expr x);
Expr  load_file(char* fname);
void  repl(void);

#endif
