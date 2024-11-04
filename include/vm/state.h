#ifndef rl_vm_state_h
#define rl_vm_state_h

#include "runtime.h"

/* Stores references to values that only exist on the C stack in case a garbage collection cycle occurs before an object is visible in Rascal. */
struct HFrame {
  HFrame* next;
  size64  cnt;
  Val*    saved;
};

/* Stores all globals used by the Rascal virtual machine. */
struct State {
  /* Heap state */
  HFrame*  hfs;
  Vec*     grays;
  Obj*     heap;
  size64   alloc, limit;

  /* Environment state */
  Map*     globals;  // global variables
  Map*     metadata; // metadata for immediate values
  Sym*     symbols;  // symbol table root (symbol table itself is an invasive tree)

  /* execution state */
  Process* main;
};

struct Process {
  /* link back to global state */
  State*  vm;

  /* Environment state */
  Cons*   upvals;

  /* Execution state */
  UserFn* code;
  sint16* ip;
  Val*    bp;
  Val*    fp;
  Val*    cp;

  /* Stack state */
  Val*    base;
};

/* APIs for State object. */
void rl_init_state(State* state, Process* proc);
void rl_init_process(Process* proc, State* state, Val* stack);
void rl_toplevel_init_state(void);

#endif
