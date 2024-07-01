#ifndef rl_vm_reader_h
#define rl_vm_reader_h

#include "val/array.h"
#include "val/text.h"

/* Declarations, APIs, and globals for runtime reader state object. */
/* C types */
/* Globals */
extern RT   BaseRt;
extern RT   SymRt;
extern RT   HashRt;
extern RT   TickRt;
extern MStr TBuffer;
extern MBin IBuffer;

/* External APIs */
void init_rts(void);

#endif
