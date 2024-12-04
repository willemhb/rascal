#include <string.h>

#include "labels.h"

#include "vm/state.h"
#include "vm/error.h"
#include "vm/type.h"

#include "val/function.h"
#include "val/array.h"
#include "val/table.h"

/* Globals */
/* Stacks */
Val    Stack[MAX_STACK];
EFrame Errors[MAX_ERROR];

/* Global State objects */
extern Ns       Globals;
extern Table    Meta;
extern StrTable Strings;
extern Alist    Grays;
extern VTable*  Vts[N_TYPES];

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

Proc Main = {};

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

// call stack manipulation
void pushf(Proc* p) {
  assert(p->sp + 3 < p->s_end);

  // push current execution state
  push(p, tag(p->code));
  push(p, tag(p->ip));
  push(p, tag(p->bp));
}

void popf(Proc* p) {
  assert(p->sp - 3 >= p->stk);

  p->bp   = as_ptr(*(--p->sp));
  p->ip   = as_ptr(*(--p->sp));
  p->code = as_ufn(*(--p->sp));
}

// initialization
void rl_init_state(State* vm, Proc* pr) {
  vm->main = pr;
}

void rl_init_process(Proc* pr, State* vm, Val* vs, EFrame* es) {
  pr->vm    = vm;
  pr->upvs  = NULL;

  // error state
  pr->cp    = es;
  pr->ctch  = es;
  pr->c_end = es+MAX_ERROR;

  // execution state
  pr->next  = O_NOOP;
  pr->code  = NULL;
  pr->ip    = NULL;
  pr->bp    = NULL;

  // stack state
  pr->sp    = vs;
  pr->stk   = vs;
  pr->s_end = vs+MAX_STACK;

  // initialize state object
  rl_init_state(vm, pr);
}

void rl_toplevel_init_state(void) {
  rl_init_process(&Main, &Vm, Stack, Errors);
}
