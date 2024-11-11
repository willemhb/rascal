#ifndef rl_lang_eval_h
#define rl_lang_eval_h

#include "val/value.h"

/* Toplevel eval dispatch and repl. */
Error rl_eval(State* vm, Val v, Val* b);
Error rl_repl(State* vm);

#endif
