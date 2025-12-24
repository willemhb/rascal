#ifndef rl_lang_exec_h
#define rl_lang_exec_h

#include "common.h"

// Globals --------------------------------------------------------------------
// Prototypes -----------------------------------------------------------------
Expr load_file(RlState* rls, char* fname);
Expr exec_code(RlState* rls, int argc, int flags);

#endif
