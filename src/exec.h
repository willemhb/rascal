#ifndef rl_exec_h
#define rl_exec_h

#include "rascal.h"

#include "def/opcodes.h"

/* API */
val_t  exec(module_t c);
val_t  exec_at(module_t c, opcode_t entry, uint argx);

#endif
