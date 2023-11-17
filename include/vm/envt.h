#ifndef rl_vm_envt_h
#define rl_vm_envt_h

#include "vm/context.h"

/* Globals and APIs for interacting with the global environment. */

/* External API */
UpValue* get_upval(size_t i);
void     close_upvals(size_t bp);

void vm_mark_envt(void);

/* Initialization */
void vm_init_envt(void);

#endif
