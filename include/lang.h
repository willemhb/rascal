#ifndef rl_lang_h
#define rl_lang_h

#include <stdio.h>

#include "common.h"
#include "runtime.h"
#include "data.h"

// forward declarations
Expr read_exp(FILE* in);
Expr eval_exp(Expr x);
void print_exp(FILE* out, Expr x);
void repl(void);

#endif
