#include <string.h>

#include "val/state.h"
#include "val/list.h"
#include "val/function.h"
#include "val/text.h"
#include "val/table.h"
#include "val/array.h"
#include "val/environ.h"

#include "vm/heap.h"

#include "util/number.h"

/* Globals */
// various macros
#define CL_FCNT 3

// statically allocated space for stacks/buffers
Val  IVals[MAX_STACK] = {};

Proc Main = {
  .type   = &ProcType,
  .trace  = true,
  .free   = false,
  .sweep  = false,
  .gray   = true,

  .ini    = false,

  /* Global state */
  .state  = &Vm,
  .pid    = 0,

  /* Error state */
  .err    = E_OKAY,
  .errb   = NOTHING,
  .errm   = NULL,

  /* heap state */
  .grays  = NULL,
  .hfs    = NULL,
  .heaps  = 0,
  .heapm  = INIT_HEAP,
  .gcobjs = NULL,

  /* stack state */
  .stk     = IVals,
  .top     = IVals+MAX_STACK,
  .sp      = IVals,

  /* upvalue state */
  .upvs   = NULL,

  /* interpreter state */
  .fn     = NULL,
  .nv     = NULL,

  .hp     = NULL,

  .bp     = NULL,
  .fp     = NULL
};

State Vm = {
  .type = &StateType,
  .trace  = true,
  .free   = true,
  .sweep  = false,
  .gray   = true,
  
  /* Environment state */
  .gns    = &Globals,
  .nss    = &NameSpaces,
  .strs   = &Strings,
  .md     = &MetaData,

  /* Process state */
  .pr     = &Main,

  /* Error state */
  .errkws = ErrorKws,

  /* Command line arguments */
  .args   = &EmptyList,
  .flags  = &EmptyMap,
  .opts   = &EmptyMap,

  /* Standard streams */
  .ins    = &StdIn,
  .outs   = &StdOut,
  .errs   = &StdErr
};

/* Internal APIs */
/* External APIs */
/* State APIs */
void init_rls(State* s) {
  init_gns(s, s->gns);
  init_pr(s->pr);
}

/* Proc APIs */
void init_pr(Proc* p) {
  add_to_heap(p, p);

  if ( p->pid == 0 ) { // register global state objects so they get traced correctly
    add_to_heap(p, p->state);
    add_to_heap(p, p->state->gns);
    add_to_heap(p, p->state->nss);
    add_to_heap(p, p->state->strs);
    add_to_heap(p, p->state->md);
    add_to_heap(p, p->state->args);
    add_to_heap(p, p->state->flags);
    add_to_heap(p, p->state->ins);
    add_to_heap(p, p->state->outs);
    add_to_heap(p, p->state->errs);
  }

  // initialize gray stack
  p->grays        = new_alist(p, NULL, 0, RESIZE_PYTHON);
  p->grays->trace = false;
}

void reset_pr(Proc* p) {
  // close any open upvalues
  close_upvs(&p->upvs, p->bp);

  // free stack space
  free_pr(p);

  // reset registers and flags (skip heap)
  p->err  = E_OKAY;
  p->errb = NOTHING;
  p->errm = NULL;
  p->fn   = NULL;
  p->ip   = NULL;
  p->nv   = &Globals;
  p->vs   = NULL;
  p->rp   = NULL;
  p->bp   = NULL;
  p->nx   = L_READY;
}

void trace_pr(void* o) {
  Proc* p = o;

  mark(p->state);
  mark(p->errm);
  mark(p->grays); // won't be added to itself because of trace flag
  mark(p->upvs);
  mark(p->fn);
  
  if ( p->fn != NULL && !is_proto(p->fn) )
    mark(p->nv);

  // mark stack
  for ( Val* s=p->stk; s<p->sp; s++ )
    mark(*s);

  // mark heap frames
  for (HFrame* f=p->hfs; f != NULL; f=f->next )
    for ( size_t i=0; i<f->cnt; i++ )
      mark(f->vals[i]);
}


void free_pr(void* o) {
  Proc* p = o;

  deallocate(NULL, p->stk, 0);
}

// stack helpers (unchecked, validation happens elsewhere)
Val* pr_push(Proc* p, Val x) {
  Val* o = p->sp++;
  *o     = x;

  return o;
}

Val* pr_dup(Proc* p) {
  Val* o = p->sp++;
  *o     = *(p->sp-1);

  return o;
}

Val* pr_write(Proc* p, size_t n, Val* x) {
  Val* o  = p->sp;
  p->sp  += n;

  if ( x )
    memcpy(o, x, n*sizeof(Val));

  return o;
}

Val* pr_pushn(Proc* p, size_t n, ...) {
  Val* b = pr_write(p, n, NULL);
  va_list va;
  va_start(va, n);
  
  for ( size_t i=0; i<n; i++ )
    b[i] = va_arg(va, Val);

  va_end(va);

  return b;
}

Val* pr_reserve(Proc* p, Val x, size_t n) {
  Val* b = pr_write(p, n, NULL);

  for ( size_t i=0; i<n; i++ )
    b[i] = x;

  return b;
}

Val pr_pop(Proc* p) {
  return *(--p->sp);
}

Val pr_popn(Proc* p, size_t n) {
  Val o  = *p->sp;
  p->sp -= n;

  return o;
}

Val pr_popnth(Proc* p, size_t n) {
  Val o = p->sp[-n];

  memmove(p->sp-n, p->sp-(n-1), n-1*sizeof(Val));
  p->sp--;

  return o;
}
