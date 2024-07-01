#ifndef rl_vm_state_h
#define rl_vm_state_h

#include "val/function.h"
#include "val/array.h"
#include "val/text.h"
#include "val/table.h"
#include "val/environ.h"

/* Runtime interpreter state types and APIs */
/* C types */
typedef enum FType {
  F_NONE,          // initial
  F_CLOSURE,       // user function
  F_CATCH_BODY,    // body of a catch* form
  F_CATCH_HANDLER, // handler of a catch* form
  F_HNDL_BODY,     // body of a hndl* form
  F_HNDL_HANDLER,  // handler of a hndl* form
  F_NATIVE,        // native function
  F_PRIMITIVE      // primitive function
} FType;

struct HFrame {
  RlProc* proc;
  HFrame* next;
  size_t  count;
  Val*    values;
};

struct RlProc {
  /* miscellaneous flags */
  bool     ini;
  FType    ft;

  /* link back to global state */
  RlState* state;

  /* error state */
  Error    err;        // current error
  Str*     errm;       // current error message

  /* heap state */
  Alist*   grays;        // objects waiting to be traced
  HFrame*  hfs;          // objects saved in C heap
  size_t   heaps, heapm; // heap used/maximum
  Obj*     gcobjs;       // all collectable objects

  /* Interpreter state */
  MVec*    stk;  // runtime stack (single stack machine)
  UpVal*   upvs;

  // Main registers (saved with every call)
  union {          // currently executing function
    Func*      f;
    Closure*   c;
    Native*    n;
    Primitive* p;
  } fn;

  union {
    short* ip;     // if currently executing function is a closure, this is the instruction pointer
    Env*   nv;     // if currently executing function is a builtin, this is a copy of the nearest enclosing environment
  };

  size_t    fs;    // size of the current frame (offset from TOS to base pointer)

  /* Effect registers (saved by catch/hndl forms) */
  size_t    cp; // stack address of frame saved by `catch` form
  size_t    hp; // stack address of frame saved by `hndl` form

  /* volatile registers */
  Label     lb; // current instruction or label
  size_t    bp; // first argument in current frame
  size_t    fp; // saved caller state
};

struct RlState {
  /* environment state */
  Env*     gns;
  NSMap*   nss;
  SCache*  strs;

  /* Interpreter state */
  RlProc*  proc; // currently executing process (at present there's only one of these)

  /* Error state */
  Sym**    errkws;

  /* Builtin types */
  Type**   ptypes;
};

/* External APIs */
/* RlState APIs */
void   init_rls(RlState* s);

/* RlProc APIs */
void   init_rlp(RlProc* p);
void   reset_rlp(RlProc* p);
void   mark_rlp(RlProc* p);

// pseudo-accessors
size_t rlp_sp(RlProc* p);
Val*   rlp_svals(RlProc* p, size_t o);
char*  rlp_fname(RlProc* p);
Env*   rlp_env(RlProc* p);

// stack helpers
Val    rlp_peek(RlProc* p, long i);
void   rlp_poke(RlProc* p, long i, Val x);
void   rlp_rpop(RlProc* p, bool m, size_t d, size_t n);
size_t rlp_push(RlProc* p, bool m, Val x);
size_t rlp_write(RlProc* p, bool m, Val* x, size_t n);
size_t rlp_pushn(RlProc* p, bool m, size_t n, ...);
size_t rlp_reserve(RlProc* p, bool m, Val x, size_t n);
Val    rlp_pop(RlProc* p, bool m);
Val    rlp_popn(RlProc* p, bool m, size_t n);

// frame helpers
#define push_rx(p, rx) rlp_push(p, false, tag(p->rx))

void   rlp_save_frame(RlProc* p);
void   rlp_init_frame(RlProc* p, size_t o, size_t n, Val x);
void   rlp_install_cl(RlProc* p, Closure* c, int t);

#endif
