#ifndef rl_lang_print_h
#define rl_lang_print_h

#include "common.h"

// globals --------------------------------------------------------------------
// forward declarations -------------------------------------------------------
void print_exp(Port* out, Expr x);
void print_embed(Port* p, char* pre, char* sep, char* last, char* end, int n, ...);

#endif
