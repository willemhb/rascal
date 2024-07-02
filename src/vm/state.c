#include <string.h>

#include "vm/state.h"
#include "vm/reader.h"
#include "vm/environ.h"
#include "vm/heap.h"

#include "val/list.h"
#include "val/function.h"
#include "val/text.h"
#include "val/table.h"

#include "util/number.h"

/* Globals */
// various macros
#define FN_FRAME_CNT 3

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
  /* Environment state */
  .gns  = &Globals,
  .nss  = &NameSpaces,
  .strs = &Strings,

  /* Process state */
  .proc = &Main,

  /* Command line arguments */
  .args  = &EmptyList,
  .flags = &EmptyMap,
  .opts  = &EmptyMap,

  /* Standard streams */
  .ins   = &StdIn,
  .outs  = &StdOut,
  .errs  = &StdErr
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
      rlp_grow_fs(p, m);
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
    rlp_grow_fs(p, m*n);
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
    rlp_grow_fs(p, m*n);
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
      rlp_init_frame(p, o, n, x);
      rlp_grow_fs(p, m*n);
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
      rlp_shrink_fs(p, m);
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
    rlp_shrink_fs(p, m*n);
  }

  return o;
}

void rlp_lrotn(RlProc* p, size_t n) {
  size_t sp = rlp_sp(p);

  if ( n > sp ) {
    rlp_panic(p, RUNTIME_ERROR, NULL, "requested left rotate %zu greater than stack size %zu", n, sp);
  } else if ( n < sp ) {
    // calculate buffer
    size_t l = min(n, sp-n);
    Val b[l];

    // swap values
    memcpy(b, rlp_svals(p, sp-n-l), l*sizeof(Val));
    memmove(rlp_svals(p, sp-n-l), rlp_svals(p, sp-n), n*sizeof(Val));
    memcpy(rlp_svals(p, sp-n), b, l*sizeof(Val));
  }
}

void rlp_rrotn(RlProc* p, size_t n) {
  size_t sp = rlp_sp(p);

  if ( n > sp ) {
    rlp_panic(p, RUNTIME_ERROR, NULL, "requested right rotate %zu greater than stack size %zu", n, sp);
  } else if ( n < sp ) {
    // calculate buffer
    size_t r = min(n, sp-n);
    Val b[r];

    // swap values
    memcpy(b, rlp_svals(p, 0), r*sizeof(Val));
    memmove(rlp_svals(p, 0), rlp_svals(p, sp-n), n*sizeof(Val));
    memcpy(rlp_svals(p, sp-r), b, r*sizeof(Val));
  }
}

void rlp_move(RlProc* p, size_t d, size_t s, size_t n) {
  size_t sp = rlp_sp(p);

  if ( )
}

// frame helpers
void rlp_grow_fs(RlProc* p, size_t n) {
  p->fs += n;
}

void rlp_shrink_fs(RlProc* p, size_t n) {
  p->fs -= n;
}

void rlp_resize_frame(RlProc* p, size_t ac, size_t lc, size_t fc) {
  assert(p->fn.f != NULL);

  // get current frame size parameters
  size_t cac = p->fn.f->argc;
  size_t clc = p->fn.f->lvarc;
  size_t cfc = p->fn.f->framec;

  // very optimistic case - frames happen to be the same size
  if ( cac == ac && clc == lc && cfc == fc ) {
    rlp_move(p, p->bp, rlp_sp(p)-ac, ac);
    rlp_setsp(p, false, p->bp+ac+lc+fc);
  }

  else {
    // preserve current frame values
    Val f[cfc];

    // move call arguments to base pointer
}
