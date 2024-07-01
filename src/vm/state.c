#include <string.h>

#include "vm/state.h"
#include "vm/reader.h"
#include "vm/environ.h"
#include "vm/heap.h"

#include "val/list.h"
#include "val/function.h"

/* Globals */
// statically allocated space for stacks/buffers
Val  IVals[MAX_STACK] = {};

MVec Stack = {
  .type    = &MVecType,
  .trace   = true,
  .free    = true,
  .sweep   = false,
  .gray    = true,
  
  .algo    = RESIZE_PYTHON,
  .shrink  = false,
  
  .data    = IVals,
  ._static = IVals,
  .cnt     = 0,
  .maxc    = MAX_STACK,
  .maxs    = MAX_STACK
};

RlProc Main = {
  /* Miscellaneous flags */
  .ini    = false,
  .ft     = F_NONE,

  /* Global state */
  .state  = &Vm,
  
  /* Error state */
  .err    = OKAY,
  .errm   = NULL,

  /* heap state */
  .grays  = &Grays,
  .hfs    = NULL,
  .heaps  = 0,
  .heapm  = INIT_HEAP,
  .gcobjs = NULL,

  /* Interpreter state */
  .stk    = &Stack,
  .upvs   = NULL,

  .fn.f   = NULL,
  .ip     = NULL,
  .fs     = 0,

  .cp     = 0,
  .hp     = 0,
  .lb     = OP_READY,
  .bp     = 0,
  .fp     = 0
};

RlState Vm = {
  .gns  = &Globals,
  .nss  = &NameSpaces,
  .strs = &Strings,
  .proc = &Main,
  
};

/* External APIs */
/* RlState APIs */
void init_rls(RlState* s) {
  init_globals(s->gns);
  init_rlp(s->proc);
}

/* RlProc APIs */
void init_rlp(RlProc* p) {
  (void)p;
}

void reset_rlp(RlProc* p) {
  // close any open upvalues
  close_upvs(&p->upvs, rlp_svals(p, 0));
  
  // free stack space
  free_mvec(p->stk);

  // reset registers and flags (skip heap)
  p->ft   = F_NONE;
  p->err  = OKAY;
  p->errm = NULL;
  p->fn.f = NULL;
  p->ip   = NULL;
  p->fs   = 0;
  p->cp   = 0;
  p->hp   = 0;
  p->lb   = OP_READY;
  p->bp   = 0;
  p->fp   = 0;
}

// pseudo-accessors
size_t rlp_sp(RlProc* p) {
  return p->stk->cnt;
}

Val* rlp_svals(RlProc* p, size_t o) {
  assert(o < p->stk->cnt);

  return p->stk->data + o;
}

char* rlp_fname(RlProc* p) {
  if ( p->fn.f == NULL )
    return "rascal-runtime";

  return fn_name(p->fn.f);
}

Env* rlp_env(RlProc* p) {
  if ( p->ft == F_NONE )
    return p->state->gns;

  else if ( p->ft == F_CLOSURE )
    return p->fn.c->envt;

  else
    return p->nv;
}

// stack helpers
Val rlp_peek(RlProc* p, long i) {
  if ( i < 0 )
    i += rlp_sp(p);

  if ( i < 0 || i > (long)rlp_sp(p) )
    rlp_panic(p, EVAL_ERROR, NULL, "index %ld out of bounds for stack of size %zu", i, rlp_sp(p));

  return p->stk->data[i];
}

void rlp_poke(RlProc* p, long i, Val x) {
  if ( i < 0 )
    i += rlp_sp(p);

  if ( i < 0 || i > (long)rlp_sp(p) )
    rlp_panic(p, EVAL_ERROR, NULL, "index %ld out of bounds for stack of size %zu", i, rlp_sp(p));

  p->stk->data[i] = x;
}

size_t rlp_push(RlProc* p, bool m, Val x) {
  size_t o = rlp_sp(p);

  if ( o == MAX_ARITY ) {
      rlp_panic(p, RUNTIME_ERROR, NULL, "stack overflow");
      o = 0;
    } else {
      mvec_push(p->stk, x);

      if ( m )
        p->fs++;
    }

  return o;
}

size_t rlp_write(RlProc* p, bool m, Val* x, size_t n) {
  size_t o = rlp_sp(p);

  if ( o + n > MAX_ARITY ) {
    rlp_panic(p, RUNTIME_ERROR, NULL, "stack overflow");
    o = 0;
  } else {
    write_mvec(p->stk, x, n);

    if ( m )
      p->fs += n;
  }

  return o;
}

size_t rlp_pushn(RlProc* p, bool m, size_t n, ...) {
  size_t o = rlp_sp(p);

  if ( o + n > MAX_ARITY ) {
    rlp_panic(p, RUNTIME_ERROR, NULL, "stack overflow");
    o = 0;
  } else {
    va_list va;
    va_start(va, n);
    mvec_pushv(p->stk, n, va);
    va_end(va);

    if ( m )
      p->fs += n;
  }

  return o;
}

size_t rlp_reserve(RlProc* p, bool m, Val x, size_t n) {
  size_t o = rlp_sp(p);

  if ( n > 0 ) {
    if ( o + n > MAX_ARITY ) {
      rlp_panic(p, RUNTIME_ERROR, NULL, "stack overflow");
      o = 0;
    } else {
      write_mvec(p->stk, NULL, n);
      Val* b = rlp_svals(p, o);
      
      for ( size_t i=0; i<n; i++ )
        b[i] = x;

      if ( m )
        p->fs += n;
    }
  }

  return o;
}

Val rlp_pop(RlProc* p, bool m) {
  Val o;

    if ( rlp_sp(p) == 0 ) {
      rlp_panic(p, RUNTIME_ERROR, NULL, "stack underflow");
      o = NOTHING;
    } else {
      o = mvec_pop(p->stk);

      if ( m )
        p->fs++;
    }

  return o;
}

Val rlp_popn(RlProc* p, bool m, size_t n) {
  Val o;

  if ( rlp_sp(p) < n ) {
    rlp_panic(p, RUNTIME_ERROR, NULL, "stack underflow");
    o = NOTHING;
  } else {
    o = mvec_popn(p->stk, n, true);

    if ( m )
      p->fs -= n;
  }

  return o;
}

// frame helpers
void rlp_save_frame(RlProc* p) {
  if ( p->ft != F_NONE ) {
    if ( p->ft < F_HNDL_HANDLER ) {
      push_rx(p, fs);
      push_rx(p, ip);
      push_rx(p, fn.c);
    } else {
      push_rx(p, fs);
      push_rx(p, nv);
      push_rx(p, fn.f);
    }
  }
}

void rlp_init_frame(RlProc* p, size_t o, size_t n, Val x) {
  Val* b = rlp_svals(p, o);

  for ( size_t i=0; i<n; i++ )
    b[i] = x;
}

void rlp_install_cl(RlProc* p, Closure* c, int t) {
  // arguments are on stack and validated
  // t is tail call flag. -1 = not a tail call, 0 = general tail call, 1 = self call
  if ( t == -1 ) { // not a tail call
    p->fs -= c->argc;                         // remove arguments from calling frame (they now belong to closure)
    rlp_reserve(p, false, NOTHING, c->lvarc);
    rlp_save_frame(p);

  } else if ( t == 0 ) { // general tail call - extra work needs to be done to ensure frame is properly sized

  } else { // tail recursive call - can assume environment part of the frame is properly sized
    rlp_rpop(p, false, p->bp, c->argc);

    if ( p->fs > fn_fsize(c) ) // remove extra values from the stack
      rlp_popn(p, false, p->fs - fn_fsize(c) );

    rlp_init_frame(p, p->bp+c->argc, c->lvarc, NOTHING);
  }

  // install registers
  p->fn.c = c;
  p->ip   = cls_ip(c);
  p->fs   = fn_fsize(c);
}
