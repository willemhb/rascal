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
  word_t ini : 1;

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
  UpVal*   upvs;

  /* Main registers (saved with every call) */
  union {          // currently executing function
    Func*   fn;
    Proto*  cl;
    PrimFn* pf;
  };

  short*   ip;
  size_t   fs;    // size of the current frame (offset from TOS to base pointer)

  /* Effect registers (saved by `catch` and `hndl` forms) */
  Val*     hp;    // stack address of frame saved by `hndl` form

  /* volatile registers */
  Val*     bp;
  Val*     fp;
  Env*     nv;

  /* label registers */
  Label    op, nx;
};

struct State {
  HEADER;
  
  // data fields
  /* environment state */
  Env*    gns;
  NSMap*  nss;
  SCache* strs;
  MMap*   md;   // value metadata

  /* Process state */
  Proc* pr; // currently executing process (at present there's only one of these)

  /* Error state */
  Sym** errkws;

  /* Command line arguments */
  List* args;
  Set*  flags;
  Map*  opts;

  /* Standard streams */
  Port* ins;
  Port* outs;
  Port* errs;
};

/* Globals */
extern Type StateType, ProcType;

/* External APIs */
/* RlState APIs */
void init_rs(State* s);

/* RlProc APIs */
void   init_pr(Proc* p);
void   reset_pr(Proc* p);
void   trace_pr(void* o);
void   free_pr(void* o);

// pseudo-accessors
char*  pr_fname(Proc* p);
Env*   pr_env(Proc* p);
size_t pr_argc(Proc* p);
size_t pr_lvarc(Proc* p);
size_t pr_fs(Proc* p);
Val*   pr_sp(Proc* p);
Val*   pr_bp(Proc* p);
Val*   pr_fp(Proc* p);
Val*   pr_hp(Proc* p);

// frame helpers
void   pr_growf(Proc* p, size_t n);
void   pr_shrinkf(Proc* p, size_t n);
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

// frame synchronized stack helpers
Val*   pr_fpush(Proc* p, Val x);
Val*   pr_fdup(Proc* p);
Val*   pr_fwrite(Proc* p, size_t n, Val* x);
Val*   pr_fpushn(Proc* p, size_t n, ...);
Val*   pr_freserve(Proc* p, size_t n, Val x);
Val    pr_fpop(Proc* p);
Val    pr_fpopn(Proc* p, size_t n);
Val    pr_fpopnth(Proc* p, size_t n);

// other stack helpers
void   pr_move(Proc* p, Val* d, Val* s, size_t n);

#endif
