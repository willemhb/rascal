#include <string.h>

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
  .vm    = &Vm,

  /* Environment state */
  .upvs  = NULL,

  /* Error state */
  .cp    = Errors,
  .ctch  = Errors,
  .c_end = &Errors[MAX_ERROR],

  /* Execution state */
  .code  = NULL,
  .ip    = NULL,
  .bp    = NULL,

  /* Stacks */
  .sp    = Stack,
  .stk   = Stack,
  .s_end = &Stack[MAX_STACK],
};

/* APIs for State object. */
// stack manipulation
Val* push(Proc* p, Val x) {
  assert(p->sp < p->s_end);

  *p->sp = x;     // add to stack
  return p->sp++; // increment stack pointer and return location
}

Val* pushn(Proc* p, size32 n, ...) {
  // unpack variadic arguments and pass to writen
  va_list va;
  va_start(va, n);
  Val buf[n];

  for ( size32 i=0; i < n; i++ )
    buf[i] = va_arg(va, Val);

  va_end(va);

  return writen(p, n, buf);
}

Val* writen(Proc* p, size32 n, Val* s) {
  assert(p->sp + n < p->s_end);

  Val* d = p->sp; p->sp += n;

  memcpy(d, s, n*sizeof(Val));

  return d;
}

Val pop(Proc* p) {
  assert(p->sp > p->stk);

  return *(--p->sp);
}

Val popn(Proc* p, size32 n, bool e) {
  assert(p->sp + n >= p->sp);

  // fetch return value and update stack pointer
  Val o = *(e ? p->sp-1 : p->sp-n); p->sp -= n;

  return o;
}

void rl_init_state(State* vm, Proc* pr);
void rl_init_process(Proc* pr, State* vm, Val* vs, EFrame* es);

void rl_toplevel_init_state(void) {
  rl_init_state(&Vm, &Main);
}
