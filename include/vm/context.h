#ifndef rl_vm_context_h
#define rl_vm_context_h

#include "val/alist.h"

/* shared global state object required by one rascal vm instance. */
typedef struct GcFrame GcFrame;

struct GcFrame {
  GcFrame* next;
  Value*   data;
  size_t   cnt;
};

struct RlCtx {
  // heap state
  size_t   heap_size;
  size_t   heap_cap;
  GcFrame* gcframes;
  Objects  grays;
  Obj*     objects;

  // execution state
  
};

#endif
