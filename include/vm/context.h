#ifndef rl_vm_context_h
#define rl_vm_context_h

#include "val/alist.h"

/* shared global state object required by one rascal vm instance. */
typedef struct GcFrame    GcFrame;
typedef struct ErrorFrame ErrorFrame;
typedef struct ReadFrame  ReadFrame;
typedef struct ExecFrame  ExecFrame;


struct GcFrame {
  GcFrame* next;
  Value*   data;
  size_t   cnt;
};

struct ExecFrame {
  Closure*  code;  // code being executed
  Chunk*    guard; // guard handler
  uint16_t* ip;    // instruction pointer
  size_t    bp;    // base pointer
  size_t    cp;    // catch pointer
};

struct RlCtx {
  // heap state
  size_t   heap_size;
  size_t   heap_cap;
  GcFrame* gcframes;
  Obj*     objects;
  Objects* grays;

  // environment state
  MutDict* globals;

  // reader state
  Stream*  input;

  // compiler state

  // execution state
  
};

/* external API */

#endif
