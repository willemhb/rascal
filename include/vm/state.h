#ifndef rl_vm_state_h
#define rl_vm_state_h

#include "runtime.h"

/* Stores references to values that only exist on the C stack in case a garbage collection cycle occurs before an object is visible in Rascal. */
struct HFrame {
  HFrame* next;
  State*  vm;
  size64  cnt;
  Val*    saved;
};

struct EFrame {
  /* Saved call state */
  UserFn* code;
  sint16* ip;
  Val*    bp;
  Val*    sp;
  void**  fp;

  /* saved error state */
  bool user_ch;

   union {
    ErrorFn n;
    UserFn* u;
  } ch;

  jmp_buf Cstate;
};

/* Stores all globals used by the Rascal virtual machine. */
struct State {
  /* Heap state */
  HFrame*  hfs;
  Alist*   grays;
  Obj*     heap;
  size64   alloc, limit;

  /* Environment state */
  Table*    globals;  // global variables
  Table*    meta;     // metadata for immediate values
  StrTable* strs;     // table of interned strings
  VTable*   vts;

  /* execution state */
  Proc*     main;
};

struct Proc {
  /* link back to global state */
  State*  vm;

  /* Environment state */
  Upv*   upvals;

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
void rl_init_state(State* vm, Proc* pr);
void rl_init_process(Proc* pr, State* vm, Val* vs, EFrame* es);
void rl_toplevel_init_state(void);

#endif
