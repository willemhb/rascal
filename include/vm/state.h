#ifndef rl_vm_state_h
#define rl_vm_state_h

#include "runtime.h"

/* Stores references to values that only exist on the C stack in case a garbage collection cycle occurs before an object is visible in Rascal. */
struct HFrame {
  HFrame* next;
  size64  cnt;
  Val*    saved;
};

/* Stores all the information necessary to resume an execution context. */
struct EFrame {
  Func*   handler;
  HFrame* hfs;
  Table*  module;
  Func*   code;
  sint16* ip;
  Val*    fp, * bp, * sp;
  jmp_buf Cstate;
};

/* Stores all internal state used by the Rascal virtual machine. */
struct State {
  /* Heap state */
  HFrame* hfs;
  Alist*  grays;
  Obj*    objs;
  size64  alloc, limit;

  /* Error state */
  EFrame* ep, * ebase, * elast;

  /* Environment state */
  UpVal* upvals;  // linked list of open upvalues
  Table* globals; // global variables
  Table* module;  // variables for currently executing script (NULL in repl)
  Sym*   symbols; // symbol table root (symbol table is invasive tree)

  /* Execution state */
  Func*   code;
  sint16* ip;
  Val*    fp, * bp;

  /* stack state */
  Val*    sp, * sbase, * slast;
};

/* APIs for State object. */
void rl_init_state(State* state, Alist* grays, Table* globals, Val* stack, EFrame* errors);
void rl_toplevel_init_state(void);

#endif
