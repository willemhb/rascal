#ifndef rl_lang_eval_h
#define rl_lang_eval_h

#include "val/value.h"

/* Toplevel eval dispatch and repl. */

rl_err_t rl_eval(Val v, Env* e, Val* b);
rl_err_t rl_repl(void);


#endif
