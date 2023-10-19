#ifndef rascal_eval_h
#define rascal_eval_h

#include "opcodes.h"
#include "runtime.h"

// external API
Value  apply_cl(Closure* cl, size_t n);
Value  load(char* path);
Value  use_module(char* path);
Value  exec(Closure* code);
Value  eval(Value val);
void   repl(void);

#endif
