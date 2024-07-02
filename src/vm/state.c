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
#define CL_FCNT 3

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

  .fn     = NULL,
  .nv     = &Globals,
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

/* Internal APIs */
static bool rlp_resize_frame(RlProc* p, size_t ac, size_t lc, size_t fc) {
    size_t nfs = ac + lc + fc;
    size_t cac = p->fn->argc;
    size_t clc = p->fn->lvarc;
    size_t cfc = p->fn->framec;
    size_t csp = rlp_sp(p);

    bool o;

    if ( cac == ac && clc == lc && cfc == fc ) { // treat like tail recursive call
      rlp_move(p, p->bp, csp-ac, ac);
      rlp_init_spc(p, NOTHING, p->bp+ac, lc);
      rlp_setsp(p, false, p->bp+nfs);

      o = false;
    } else {
      // save current frame
      Val b[fc];
      memcpy(b, rlp_svals(p, p->fp), fc*sizeof(Val));

      // move arguments to base position
      rlp_move(p, p->bp, csp-ac, ac);

      // discard frame and other temporaries, reserve space for locals, reinstall calling frame
      rlp_setsp(p, false, p->bp+ac);
      rlp_reserve(p, false, NOTHING, lc);
      rlp_write(p, false, b, fc*sizeof(Val));

      o = true;
    }

    return o;
}

static int rlp_install_args(RlProc* p, Func* f, int t) {
  // NB: function arguments are already in correct position on stack
  size_t ac = f->argc;
  size_t lc = f->lvarc;
  size_t fc = CL_FCNT;
  size_t fs = ac + lc + fc;
  int o = t;

  if ( t == -1 ) { // Not a tail call - save calling frame
    rlp_shrink_fs(p, ac);               // deregister arguments from caller's frame
    rlp_reserve(p, false, NOTHING, lc); // push local function variables
    rlp_save_frame(p);                  // save caller state
  } else if ( t == 0 ) { // general tail call - frame may need to be resized
    o = rlp_resize_frame(p, ac, lc, fc) ? -1 : 0;

  } else { // tail recursive call - frame resize not needed
    size_t csp = rlp_sp(p);

    rlp_move(p, p->bp, csp-ac, ac);          // move call arguments to base position
    rlp_init_spc(p, NOTHING, p->bp+ac, lc);  // initialize other locals
    rlp_setsp(p, false, p->bp+fs);           // remove other arguments and temporaries
  }

  return o;
}

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
  p->fn   = NULL;
  p->nv   = &Globals;
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

Val* rlp_base(RlProc* p) {
  return p->stk->data + p->bp;
}

Val* rlp_frame(RlProc* p) {
  return p->stk->data + p->fp;
}

char* rlp_fname(RlProc* p) {
  if ( p->fn == NULL )
    return "rascal-runtime";

  return fn_name(p->fn);
}

Env* rlp_env(RlProc* p) {
  if ( p->ft == F_NONE )
    return p->state->gns;

  else if ( p->ft == F_CLOSURE )
    return p->cl->envt;

  else
    return p->nv;
}

// stack helpers
size_t rlp_grow_stk(RlProc* p, bool m, size_t n) {
  size_t o;
  
  if ( rlp_sp(p) + n > MAX_ARITY ) {
    rlp_panic(p, EVAL_ERROR, NULL, "stack overflow");
    o = 0;
  } else {
    grow_mvec(p->stk, n);
    rlp_grow_fs(p, m*n);
    o = rlp_sp(p);
  }

  return o;
}

size_t rlp_shrink_stk(RlProc* p, bool m, size_t n) {
  size_t o;

  if ( n > rlp_sp(p) ) {
    rlp_panic(p, EVAL_ERROR, NULL, "stack underflow");
    o = 0;
  } else {
    shrink_mvec(p->stk, n);
    rlp_shrink_fs(p, m*n);
    o = rlp_sp(p);
  }

  return o;
}

void rlp_setsp(RlProc* p, bool m, size_t n) {
  size_t s = rlp_sp(p);
  
  if ( n < s )
    rlp_shrink_stk(p, m, s-n);

  else if ( n > s )
    rlp_grow_stk(p, m, n-s);
}

void rlp_init_spc(RlProc* p, Val x, size_t o, size_t n) {
  Val* s = rlp_svals(p, o);

  for ( size_t i=0; i<n; i++ )
    s[i] = x;
}

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
      rlp_init_spc(p, x, o, n);
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
  static const char* fmt =
    "requested move of %zu items"
    " from %zu to %zu out of bounds"
    " for stack of size %zu";

  size_t sp = rlp_sp(p);

  if ( d + n > sp )
    rlp_panic(p, RUNTIME_ERROR, NULL, fmt, n, d, s, sp);
  else
    memmove(rlp_svals(p, d), rlp_svals(p, s), n*sizeof(Val));
}

// frame helpers
void rlp_grow_fs(RlProc* p, size_t n) {
  p->fs += n;
}

void rlp_shrink_fs(RlProc* p, size_t n) {
  p->fs -= n;
}

void rlp_save_frame(RlProc* p) {
  if ( p->ft < F_NATIVE ) {
    push_rx(p, fs);
    push_rx(p, ip);
    push_rx(p, cl);
  } else {
    push_rx(p, fs);
    push_rx(p, nv);
    push_rx(p, fn);
  }
}

void rlp_install_fn(RlProc* p, Func* f, int t) {
  if ( f->ft < F_NONE )
    rlp_install_proto(p, (Proto*)f, t);

  else {
    Env* n     = rlp_env(p);
    size_t ac  = f->argc;
    size_t lc  = f->lvarc;
    size_t fc  = f->framec;
    size_t fs  = ac + lc + fc;
    t          = rlp_install_args(p, f, t);
    size_t csp = rlp_sp(p);

    // install registers (which registers need to be updated depends on whether frame was resized)
    if ( t == -1 ) {
      p->fn = f;
      p->nv = n;
      p->fs = fs;
      p->bp = csp - fs;
      p->fp = csp - fc;
      p->ft = f->ft;
    } else if ( t == 0 ) {
      p->fn = f;
      p->nv = n;
      p->fs = fs;
      p->ft = f->ft;
    } else {
      p->fs = fs;
    }
  }
}

void rlp_install_proto(RlProc* p, Proto* c, int t) {
  // NB: function arguments are already in correct position on stack
  size_t ac = c->argc;
  size_t lc = c->lvarc;
  size_t fc = CL_FCNT;
  size_t fs = ac + lc + fc;

  t = rlp_install_args(p, (Func*)c, t);

  // install registers (what needs to be computed depends on t and whether the frame was resized)
  if ( t == -1 ) { // Not a tail call - save calling frame
    size_t csp = rlp_sp(p);

    p->cl = c;
    p->ip = proto_ip(c);
    p->fs = fs;
    p->bp = csp - fs;
    p->fp = csp;
    p->ft = c->ft;
  } else if ( t == 0 ) { // general tail call, frame not resized
    p->cl = c;
    p->ip = proto_ip(c);
    p->fs = fs;
    p->ft = c->ft;
  } else { // tail recursive call - frame resize not needed
    // install registers
    p->ip = proto_ip(c);
    p->fs = fs;
  }
}

void rlp_install_catch(RlProc* p, Proto* b, Proto* h) {
  // install body (this saves caller state)
  rlp_install_proto(p, b, -1);
  // install handler and current catch pointer
  rlp_push(p, true, tag(h));
  rlp_push(p, true, tag(p->cp));
  // set frame pointer and catch pointer
  p->fp = rlp_sp(p);
  p->cp = rlp_sp(p);
}

void rlp_install_hndl(RlProc* p, Proto* b, Proto* h) {
  // install body (this saves caller state)
  rlp_install_proto(p, b, -1);
  // install handler and current hndl pointer
  rlp_push(p, true, tag(h));
  rlp_push(p, true, tag(p->hp));
  // set frame pointer and hndl pointer
  p->fp = rlp_sp(p);
  p->hp = rlp_sp(p);
}
