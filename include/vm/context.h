#ifndef rl_vm_context_h
#define rl_vm_context_h

#include "val/object.h"

/* Globals and APIs for interacting with the global environment. */

/* C types */

/* Specialized structure used to store interned strings.

   Strings up to a certain size are interned. */

typedef struct {
  String** table;
  size_t cnt;
  size_t cap;
} StringTable;


/* External API */
UpValue* get_upval(size_t i);
void     close_upvals(size_t bp);

void vm_mark_envt(void);

/* Initialization */
void vm_init_envt(void);

#endif
