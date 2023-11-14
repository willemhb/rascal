#ifndef rl_vm_context_h
#define rl_vm_context_h

#include <setjmp.h>

#include "val/alist.h"

/* shared global state object required by one rascal vm instance. */
typedef struct GcFrame   GcFrame;
typedef struct ErrFrame  ErrFrame;
typedef struct ExecFrame ExecFrame;

struct GcFrame {
  GcFrame* next;
  Value*   data;
  size_t   cnt;
};

struct ErrFrame {
  ErrFrame* next;

  // saved heap state
  GcFrame*  frames;

  // 

  // saved execution state
  Closure*  code;
  Chunk*    effh;
  uint16_t* ip;
  size_t    bp;
  size_t    cp;
  size_t    sp;
  size_t    fp;

  // saved C context
  jmp_buf   Cstate;
};

struct ExecFrame {
  Closure*  code;  // code being executed
  Chunk*    effh;  // effect handler
  uint16_t* ip;    // instruction pointer
  size_t    bp;    // base pointer
  size_t    cp;    // catch pointer (address of enclosing effect handler)
};

struct RlCtx {
  // heap state
  size_t      heap_size;
  size_t      heap_cap;
  GcFrame*    gcframes;
  Obj*        objects;
  Objects*    grays;

  // environment state
  MutDict*    globals;     // global variables
  MutDict*    meta;        // value metadata

  // reader state
  MutDict*    readt;
  Stream*     input;
  Buffer8*    buffer;
  Alist*      readstk;

  // compiler state
  Chunk*      compiling;

  // execution state
  Closure*    code;
  Chunk*      effh;
  uint16_t*   ip;
  size_t      bp;
  size_t      cp;
  size_t      sp;
  size_t      fp;

  // error state
  ErrFrame*   err;
};

/* globals */
extern Value     ValueStack[];
extern ExecFrame CallStack[];

/* external API */

#endif
