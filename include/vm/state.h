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
  F_CATCH,         // body of a catch* form
  F_THROW,         // handler of a catch* form
  F_HNDL,          // body of a hndl* form
  F_RAISE,         // handler of a hndl* form
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
  MVec*    stk;          // value stack and call stack (single stack machine)
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
  Env*    gns;
  NSMap*  nss;
  SCache* strs;

  /* Interpreter state */
  RlProc* proc; // currently executing process (at present there's only one of these)

  /* Error state */
  Sym**   errkws;

  /* Command line arguments */
  List*   args;
  Set*    flags;
  Map*    opts;

  /* Standard streams */
  Port*   ins;
  Port*   outs;
  Port*   errs;
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
size_t rlp_grow_stk(RlProc* p, bool m, size_t n);
size_t rlp_shrink_stk(RlProc* p, bool m, size_t n);
Val    rlp_peek(RlProc* p, long i);
void   rlp_poke(RlProc* p, long i, Val x);
void   rlp_setsp(RlProc* p, bool m, size_t n);
size_t rlp_push(RlProc* p, bool m, Val x);
size_t rlp_write(RlProc* p, bool m, Val* x, size_t n);
size_t rlp_pushn(RlProc* p, bool m, size_t n, ...);
size_t rlp_reserve(RlProc* p, bool m, Val x, size_t n);
size_t rlp_reserve_at(RlProc* p, bool m, Val x, size_t n, size_t o);
Val    rlp_pop(RlProc* p, bool m);
Val    rlp_popn(RlProc* p, bool m, size_t n);
void   rlp_lrotn(RlProc* p, size_t n);
void   rlp_rrotn(RlProc* p, size_t n);
void   rlp_move(RlProc* p, size_t d, size_t s, size_t n);

// frame helpers
#define push_rx(p, rx)     rlp_push(p, false, tag(p->rx))
#define save_rx(p, rx, o)  rlp_poke(p, o, tag(p->rx))
#define pop_rx(p, rx)      p->rx = untag(p->rx, rlp_pop(p, false))

void   rlp_grow_fs(RlProc* p, size_t n);
void   rlp_shrink_fs(RlProc* p, size_t n);
void   rlp_resize_frame(RlProc* p, size_t ac, size_t lc, size_t fc);
void   rlp_instal_cl(RlProc* p, Closure* cl, int t);

#endif
