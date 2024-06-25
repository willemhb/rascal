#ifndef rl_vm_reader_h
#define rl_vm_reader_h

#include "val/array.h"
#include "val/text.h"

/* Declarations, APIs, and globals for runtime reader state object. */
/* C types */
typedef enum {
  RF_READY,
  RF_EXPRESSION,
  RF_EOF,
  RF_ERROR
} RFlag;

struct RState {
  Alist frames;
  MVec  stack;
  MStr  buffer;
  RFlag flag;
  RT*   rt;
  MMap* gs;
  Port* in;
};

/* Globals */
extern RT BaseRt;
extern RT SymRt;
extern RT HashRt;
extern RT TickRt;

#endif
