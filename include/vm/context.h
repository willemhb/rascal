#ifndef rl_vm_context_h
#define rl_vm_context_h

#include <setjmp.h>

#include "val/hamt.h"

/* shared global state object required by one rascal vm instance. */
typedef struct Heap        Heap;
typedef struct Reader      Reader;
typedef struct Compiler    Compiler;
typedef struct Interpreter Interpreter;

typedef struct GcFrame     GcFrame;
typedef struct ErrFrame    ErrFrame;
typedef struct ReadFrame   ReadFrame;
typedef struct CompFrame   CompFrame;
typedef struct ExecFrame   ExecFrame;

struct Heap {
  size_t   size;
  size_t   cap;
  GcFrame* frames;
  Obj*     objects;
  Objects* grays;
};

struct Reader {
  MutDict* rt;
  MutDict* gs;
  MutStr*  buf;
  Stream*  src;
  size_t   sp;
  size_t   fp;
};

struct Compiler {
  Envt*   envt;
  MutBin* code;
  MutVec* vals;
  size_t  sp;
  size_t  fp;
};

struct Interpreter {
  Closure*  code;
  Chunk*    effh;
  uint16_t* ip;
  size_t    bp;
  size_t    cp;
  size_t    sp;
  size_t    fp;
};

struct GcFrame {
  GcFrame* next;
  Value*   data;
  size_t   cnt;
};

struct ErrFrame {
  ErrFrame* next;

  // saved heap state
  GcFrame*  gcframes;

  // saved reader state
  Reader      r;

  // saved compiler state
  Compiler    c;

  // saved execution state
  Interpreter i;

  // saved C context
  jmp_buf   Cstate;
};

struct ReadFrame {
  MutDict* rt;
  MutDict* gs;
  MutStr*  buf;
  Stream*  src;
};

struct CompFrame {
  Envt*   envt;
  MutBin* code;
  MutVec* vals;
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
  Heap h;

  // reader state
  Reader r;

  // compiler state
  Compiler c;

  // execution state
  Interpreter i;

  // error state
  ErrFrame*   err;

  // global environment state
  MutDict* globals;     // global variables
  MutDict* meta;        // value metadata
  UpValue* upvals;      // open upvalues
};

/* Initialization. */
void vm_init_context(void);

#endif
