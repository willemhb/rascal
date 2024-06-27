#include <string.h>

#include "vm/interpreter.h"

#include "val/list.h"
#include "val/function.h"

/* Globals */
#define FRAME_SIZE 6

Val IVals[MAX_STACK];
Val IFrms[MAX_FRAMES];

IState Interpreter = {
  .stk = {
    .type   =&MVecType,
    .trace  =true,
    .gray   =true,

    .algo   =RESIZE_NONE,

    .data   =IVals,
    ._static=IVals,
    .cnt    =0,
    .maxc   =MAX_STACK,
    .maxs   =MAX_STACK,
  },

  .frm = {
    .type   =&MVecType,
    .trace  =true,
    .gray   =true,

    .algo   =RESIZE_NONE,

    .data   =IFrms,
    ._static=IFrms,
    .cnt    =0,
    .maxc   =MAX_FRAMES,
    .maxs   =MAX_FRAMES,
  },

  .upv=NULL,

  .ex =NULL,
  .ip =NULL,
  .fs =-1,
  .co =-1,
  .ho =-1,
  .sp =-1
};

/* Internal APIs */
void is_savef(IState* s, Val* f) {
  s->ex = as_cls(f[0]);
  s->ip = as_ptr(f[1]);
  s->fs = as_small(f[2]);
  s->co = as_small(f[3]);
  s->ho = as_small(f[4]);
  s->sp = as_small(f[5]);

}

void is_restoref(IState* s, Val* b) {
  b[0]   = tag(s->ex);
  b[1]   = tag(s->ip);
  b[2]   = tag(s->fs);
  b[3]   = tag(s->co);
  b[4]   = tag(s->ho);
  b[5]   = tag(s->sp);
}

/* External APIs */
// miscellaneous stack manipulation helpers
int  is_sp(IState* s) {
  return s->stk.cnt;
}

int is_fp(IState* s) {

  return s->frm.cnt;
}

Val* is_peeks(IState* s, int i) {
  if ( i < 0 )
    i += is_sp(s);

  return &s->stk.data[i];
}

Val* is_peekf(IState* s, int i) {
  i *= FRAME_SIZE;

  if ( i < 0 )
    i += is_fp(s);

  return &s->frm.data[i];
}

int is_push(IState* s, Val x) {
  mvec_push(&s->stk, x);
  s->fs++;

  return s->stk.cnt-1;
}

int is_pushn(IState* s, int n, Val x) {
  int o = s->stk.cnt;

  if ( tag_of(x) == CPTR )

    write_mvec(&s->stk, as_ptr(x), n);

  else {
    write_mvec(&s->stk, NULL, n);

    for ( int i=0; i<n; i++ )
      s->stk.data[o+i] = x;
  }

  return o;
}

Val is_pop(IState* s) {
  s->fs--;
  return mvec_pop(&s->stk);
}

Val is_popn(IState* s, int n, bool e) {
  s->fs -= n;
  return mvec_popn(&s->stk, n, e);
}

// frame manipulation helpers
void is_pushf(IState* s, Closure* c) {
  /* Assumes correct number of formal arguments are already on the stack and `c` has been removed. */
  /* subtract function arguments from current frame (they belong to the new frame). */
  s->fs -= c->nargs;
  
  /* reserve space, get buffer */
  int o  = write_mvec(&s->frm, NULL, 6);
  Val* b = &s->frm.data[o];

  /* save existing registers */
  is_savef(s, b);

  /* install new registers or update old ones */
  s->ex  = c;
  s->ip  = cls_ip(c);
  s->fs  = c->nargs;
  s->co  = s->co < 0 ? s->co : s->co + FRAME_SIZE;
  s->ho  = s->ho < 0 ? s->ho : s->ho + FRAME_SIZE;
  s->sp  = -1;

  /* Reserve space for local variables */
  is_pushn(s, c->nvars, NUL);

  /* Set base pointer */
  s->bp = is_peeks(s, -s->fs);
}

void is_tpushf(IState* s, Closure* c) {
  /* Reuse current frame */
  /* Move arguments into locals position */
  memmove(s->bp, is_peeks(s, -c->nargs), c->nargs*sizeof(Val));

  /* Resize frame */
  if ( s->fs > (int)c->nregs )
      is_popn(s, s->fs-c->nregs, true);

  else if ( s->fs < (int)c->nregs )
    is_pushn(s, c->nregs-s->fs, NUL);

  /* install closure */
  s->ex = c;
  s->ip = cls_ip(c);
}

void is_pushc(IState* s, Closure* c, Closure* h) {
  int sp = is_sp(s);

  is_push(s, tag(h));
  is_pushf(s, c);

  s->co = 0;
  s->sp = sp;
}

void is_pushh(IState* s, Closure* c, Closure* h) {
  int sp = is_sp(s);

  is_push(s, tag(h));
  is_pushf(s, c);

  s->ho = 0;
  s->sp = sp;
}

void is_popf(IState* s) {
  // get frame to restore and return value
  Val r = is_popn(s, s->fs, true), * f = is_peekf(s, -FRAME_SIZE);

  /* restore registers */
  is_restoref(s, f);

  /* push return value */
  is_push(s, r);

  /* restore base pointer */
  s->bp = is_peeks(s, -s->fs);
}

void is_popc(IState* s, int n) {
  assert(s->co > -1);

  Val buf[n];
  
  memcpy(buf, is_peeks(s, -n), n*sizeof(Val));

  if ( s->co > 0 ) {
    Val* f = is_peekf(s, -s->co);

    /* restore registers */
    is_restoref(s, f);
  }

  int sp     = s->sp;
  int diff   = is_sp(s) - sp;
  Closure* h = as_cls(s->bp[0]);

  /* Restore stack to saved state */
  if ( diff > 0 )
    is_popn(s, diff, true);

  s->sp = -1; // invalidate sp, since throw has been used

  /* Install handler */
  is_pushn(s, n, tag((Ptr)buf));
  is_tpushf(s, h);
}

void is_poph(IState* s, int n) {
  
}
