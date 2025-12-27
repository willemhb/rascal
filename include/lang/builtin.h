#ifndef rl_lang_builtin_h
#define rl_lang_builtin_h

#include "common.h"

/* separate module for initializing builtin functions */

// Globals --------------------------------------------------------------------
extern Fun* ReplFun, *LoadFun;

// Prototypes -----------------------------------------------------------------
void define_builtins(void);

#endif
