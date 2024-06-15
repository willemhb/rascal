#ifndef rl_lang_compile_h
#define rl_lang_compile_h

#include "val/object.h"

rl_err_t rl_compile_toplevel(List* f, Env* e, Closure** b);
rl_err_t rl_compile_script(const char* f, Closure** b);
rl_err_t rl_compile_ns(const char* ns, Closure** b );

#endif
