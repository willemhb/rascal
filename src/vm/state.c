#include "labels.h"

#include "vm/state.h"
#include "vm/error.h"
#include "vm/type.h"

#include "val/array.h"
#include "val/table.h"

/* Globals */
/* Stacks */
Val    Stack[MAX_STACK];
void*  Frames[MAX_FRAMES];
EFrame Errors[MAX_ERROR];

/* Global State objects */
extern Ns       Globals;
extern Table    Meta;
extern StrTable Strings;
extern Alist    Grays;
extern VTable   Vts[N_TYPES];

State Vm = {
  /* Heap state */
  .hfs     = NULL,
  .grays   = &Grays,
  .heap    = NULL,
  .limit   = INIT_HEAP,

  /* Environment state */
  .globals = &Globals,
  .meta    = &Meta,
  .strs    = &Strings,
  .vts     = Vts,

  /* Execution state */
  .main    = &Main,
};

Proc Main = {
  /* link back to global state */
  .vm          = &Vm,

  /* Environment state */
  .upvs        = NULL,

  /* Error state */
  .cp          = Errors,
  .catches     = Errors,
  .catches_end = &Errors[MAX_ERROR],

  /* Execution state */
  .code        = NULL,
  .ip          = NULL,
  .bp          = NULL,

  /* Stacks */
  .sp          = Stack,
  .stack       = Stack,
  .stack_end   = &Stack[MAX_STACK],

  .fp          = Frames,
  .frames      = Frames,
  .frames_end  = &Frames[MAX_FRAMES]
};

/* APIs for State object. */
void rl_init_state(State* vm, Proc* pr);
void rl_init_process(Proc* pr, State* vm, Val* vs, EFrame* es);

void rl_toplevel_init_state(void) {
  rl_init_state(&Vm, &Main);
}
