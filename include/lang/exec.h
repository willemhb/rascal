#ifndef rl_lang_exec_h
#define rl_lang_exec_h

#include "val/object.h"

/* External API */
Error rl_exec(Proc* p, Proto* c, Val* b);
Error rl_load(Proc* p, const char* f);

#endif
