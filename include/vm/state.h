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
  VTable*   vts;      // type information needed by runtime

  /* execution state */
  Proc*     main;     // execution state
};

struct Proc {
  /* link back to global state */
  State*  vm;

  /* Environment state */
  Upv*    upvs;

  /* Error state */
  EFrame* cp, * catches, * catches_end;

  /* Execution state */
  UserFn* code;
  sint16* ip;
  Val*    bp;

  /* Stacks */
  Val* sp, * stack, * stack_end;
  void** fp, ** frames, ** frames_end;
};

/* APIs for State object. */
// stack manipulation
Val* push(Proc* p, Val x);
Val* pushn(Proc* p, size32 n, ...);
Val* writen(Proc* p, size32 n, Val* src);
Val  pop(Proc* p);
Val  popn(Proc* p, size32 n);
Val  popnth(Proc* p, int n);
Val  getrx(Proc* p, int n);
Val  setrx(Proc* p, int n, Val v);

// call stack manipulation
void pushf(Proc* p);
void popf(Proc* p);

// initialization
void rl_init_state(State* vm, Proc* pr);
void rl_init_process(Proc* pr, State* vm, Val* vs, EFrame* es);
void rl_toplevel_init_state(void);

#endif
