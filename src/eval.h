#ifndef eval_h
#define eval_h

#include "value.h"

// API ------------------------------------------------------------------------
Val  eval(Val x);
Val  exec(Func* closure);
void repl(void);

#endif
