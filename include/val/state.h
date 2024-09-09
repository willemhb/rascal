#ifndef rl_val_state_h
#define rl_val_state_h

#include "val/object.h"

/* Runtime interpreter state types and APIs */
/* C types */
struct HFrame {
  Proc*   proc;
  HFrame* next;
  size_t  cnt;
  Val*    vals;
};

struct Proc {
  HEADER;
  // bit fields
  word_t ini   : 1;

  // data fields
  /* global state */
  State* state;        // global state object
  uint   pid;          // process id

  /* error state */
  Error    err;        // current error
  Val      errb;       // cause of current error
  Str*     errm;       // current error message

  /* heap state */
  Alist*   grays;        // objects waiting to be traced
  HFrame*  hfs;          // objects saved in C heap
  size_t   heaps, heapm; // heap used/maximum
  Obj*     gcobjs;       // all collectable objects

  /* stack state */
  Val*     stk, *sp, *top;

  /* open upvalues */
  UpVal*   ou;

  /* Main registers (some of these are saved by non-tail calls). */
  Func*    fn;    // currently executing function (could be closure or primitive)
  short*   ip;    // instruction pointer (for compiled code)
  Env*     nv;    // active environment
  Vec*     vs;    // constant store for active function
  Val*     cp;    // catch pointer (set by `catch` forms)
  Val*     hp;    // handle pointer (sdet by `hndl` forms)
  Val*     bp;    // arguments to current function (bp[-1] contains offset to caller state)
  Label    nx;    // next vm label (used by complex primitives like read and compile that work like state machines)
};

struct State {
  HEADER;
  
  // data fields
  /* environment state */
  Env*    gns;
  SCache* strs;
  MMap*   md;   // value metadata

  /* Process state */
  Proc*   pr; // currently executing process (at present there's only one of these)

  /* Error state */
  Sym** errkws;

  /* Command line arguments */
  List* args;
  Map*  opts;

  /* Standard streams */
  Port* ins;
  Port* outs;
  Port* errs;
};

/* Globals */
/* External APIs */
/* State APIs */
void init_rs(State* s);

/* Proc APIs */
void   init_pr(Proc* p);
void   reset_pr(Proc* p);
void   trace_pr(void* o);
void   free_pr(void* o);

// pseudo-accessors
size_t pr_nstk(Proc* p);
size_t pr_nabove(Proc* p, Val* s);
size_t pr_nbelow(Proc* p, Val* s);

// frame helpers
void   pr_initf(Proc* p, Val* d, size_t n, Val x);
void   pr_pushat(Proc* p, Val* d, size_t n, Val x);
void   pr_popat(Proc* p, Val* d, size_t n);

// stack helpers
Val*   pr_push(Proc* p, Val x);
Val*   pr_dup(Proc* p);
Val*   pr_write(Proc* p, size_t n, Val* x);
Val*   pr_pushn(Proc* p, size_t n, ...);
Val*   pr_reserve(Proc* p, size_t n, Val x);
Val    pr_pop(Proc* p);
Val    pr_popn(Proc* p, size_t n);
Val    pr_popnth(Proc* p, size_t n);

// other stack helpers
void   pr_move(Proc* p, Val* d, Val* s, size_t n);

#endif
