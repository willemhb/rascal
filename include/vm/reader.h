#ifndef rl_vm_reader_h
#define rl_vm_reader_h

#include "val/array.h"
#include "val/text.h"

/* Declarations, APIs, and globals for runtime reader state object. */

/* C types */
struct RState {
  Alist frames;
  MVec  stack;
  MBin  buffer;
  RT*   rt;
  MMap* gs;
  Port* in;
};

/* Globals */
extern RState Reader;
extern RT     Rt;
extern RT     HashRt;
extern RT     TickRt;

#endif
