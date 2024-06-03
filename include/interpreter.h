#ifndef rl_interpreter_h
#define rl_interpreter_h

#include "value.h"

/* Definitions and declarations for core interpreter functions (read, eval, print, compile, exec, and repl). */
/* External APIs */
rl_status_t rl_read(Port* ios, Value* buffer);
rl_status_t rl_compile(Cons* form, Chunk** buffer);
rl_status_t rl_execute(Closure* code, Value* buffer);
rl_status_t rl_eval(Value val, Value* buffer);
rl_status_t rl_print(Port* ios, Value val);
rl_status_t rl_repl(void);

#endif
