#ifndef rl_vm_state_h
#define rl_vm_state_h

#include "runtime.h"

/* Stores all globals used by the Rascal virtual machine. */
struct State {
  /* Heap state */
  HFrame*  hfs;
  Alist*   grays;
  Obj*     heap;
  size64   alloc, limit;

  /* Environment state */
  Ns*       globals;  // global namespace
  Table*    meta;     // metadata for immediate values
  StrTable* strs;     // table of interned strings
  VTable**  vts;      // type information needed by runtime

  /* execution state */
  Proc*     main;     // execution state
};

struct Proc {
  /* link back to global state */
  State*  vm;

  /* Environment state */
  Upv*    upvs;

  /* Error state */
  EFrame* cp, * ctch, * c_end;

  /* Execution state */
  /*
   * `next` is used by some builtin functions like `apply` and `exec` to
   * avoid recursive calls to `rl_exec`. This is accomplished by setting up
   * the stack and setting the `next` parameter to an appropriate virtual machine
   * label before returning. Most of the time this should be `OP_NOOP`.
   */
  Opcode  next;
  UserFn* code;
  sint16* ip;
  Val*    bp;

  /* Stacks */
  Val* sp, * stk, * s_end;
};

/* APIs for State object. */
// stack manipulation
Val* push(Proc* p, Val x);
Val* pushn(Proc* p, size32 n, ...);
Val* writen(Proc* p, size32 n, Val* s);
Val  pop(Proc* p);
Val  popn(Proc* p, size32 n, bool e);
Val  popnth(Proc* p, int n);

// call stack manipulation
void pushf(Proc* p);
void popf(Proc* p);

// initialization
void rl_init_state(State* vm, Proc* pr);
void rl_init_process(Proc* pr, State* vm, Val* vs, EFrame* es);
void rl_toplevel_init_state(void);

#endif
