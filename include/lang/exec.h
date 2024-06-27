#ifndef rl_lang_exec_h
#define rl_lang_exec_h

#include "val/object.h"

/* External API */
rl_err_t rl_exec(Closure* c, Val* b);
rl_err_t rl_load(const char* f);

#endif
