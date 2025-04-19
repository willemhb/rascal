#ifndef rl_lang_h
#define rl_lang_h

#include <stdio.h>

#include "common.h"
#include "runtime.h"
#include "data.h"

// forward declarations
Status  read_expr(FILE* in, Expr* out);
Status  eval_expr(Expr x, Expr* out);
void    print_expr(FILE* out, Expr x);
void    repl(void);


#endif
