#ifndef rl_eval_h
#define rl_eval_h

#include "rascal.h"

/* globals */

/* API */
val_t  exec(module_t c);
val_t  eval(val_t x);
void   repl(void);

/* initialization */
void eval_init(void);

#endif
