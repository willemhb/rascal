#ifndef rl_lang_compile_h
#define rl_lang_compile_h

#include "common.h"

// globals --------------------------------------------------------------------
// forward declarations -------------------------------------------------------
Fun* toplevel_compile(RlState* rls, Expr x);
Fun* compile_file(RlState* rls, char* fname);

#endif
