#include <string.h>

#include "lang/exec.h"
#include "lang/compare.h"

#include "val/array.h"
#include "val/function.h"
#include "val/environ.h"
#include "val/text.h"
#include "val/state.h"

#include "util/number.h"

/* Internal APIs */
static inline bool truthy(Val x) {
  return x != FALSE && x != NUL;
}

static inline bool falsey(Val x) {
  return x == FALSE || x == NUL;
}

static inline UpVal** pr_upvs(Proc* p) {
  return p->cl->envt->upvals;
}

static inline Val* pr_vals(Proc* p) {
  return p->cl->vals->tail;
}

static inline UpVal** cl_upvs(Proto* p) {
  return p->envt->upvals;
}

static Ref* get_ns_ref(Proc* p, int yoff, int xoff) {
  Env* nv;
  Ref* o = NULL;

  if ( yoff == -1 ) {
    nv = pr_env(p)->ns;

    if ( has_ref(nv, xoff) )
      o = env_ref(nv, xoff);

  } else {
    nv = p->state->gns;

    if ( has_ref(nv, yoff) ) {
      o = env_ref(nv, yoff);

      if ( is_ns_ref(o) ) {
        nv = as_env(o->val);

        if ( has_ref(nv, xoff) )
          o = env_ref(nv, xoff);
        else
          o = NULL;
      } else {
        o = NULL;
      }
    }
  }

  return o;
}

static void pr_savef(Proc* p) {
  if ( is_proto(p->fn) )
    pr_pushn(p, 3, tag(p->fn), tag(p->ip), tag(p->fs));

  else
    pr_pushn(p, 3, tag(p->fn), tag(p->nv), tag(p->fs));
}

static void pr_restoref(Proc* p) {
  Val* f     = p->fp;
  size_t ofs = p->fs;

  // restore calling frame
  p->fs      = as_arity(f[-1]);
  p->ip      = as_ptr(f[-2]);
  p->fn      = as_func(f[-3]);

  // discard finished frame
  pr_popn(p, ofs);

  // update volatile registers
  p->bp      = p->sp-p->fs;
  p->fp      = p->bp + fn_fsize(p->fn);
}

static void adjust_args(Proc* p, Func* f) {
  size_t cac = p->fn->argc;
  size_t fac = f->argc;

  if ( cac < fac )
    pr_pushat(p, pr_bp(p)+cac, fac-cac, NOTHING);

  else if ( cac > fac )
    pr_popat(p, p->bp+cac, cac-fac);
}

static void adjust_lvars(Proc* p, Func* f) {
  size_t clc = pr_lvarc(p);
  size_t flc = fn_lvarc(f);
  size_t fac = fn_argc(f);

  if ( clc < flc )
    pr_pushat(p, pr_bp(p)+fac, flc-clc, NOTHING);

  else if ( clc > flc )
    pr_popat(p, pr_bp(p)+fac, clc-flc);
}

static void install_primfn(Proc* p, PrimFn* f, int t) {
  // NB: arguments to call already on stack and validated
  Env* nv = pr_env(p); // save current environment
  
  if ( t == -1 ) { // not a tail call
    pr_shrinkf(p, f->argc);           // remove arguments to f from calling frame
    pr_reserve(p, f->lvarc, NOTHING); // reserve space for other locals
    pr_savef(p);                      // save current frame

    // install registers
    p->fs = fn_fsize(f);
    p->pf = f;
    p->nv = nv;
    p->bp = pr_sp(p)-pr_fs(p);
    p->fp = pr_sp(p);

  } else if ( t == 0 ) { // general tail call (frame may need to be resized)
    // adjust size
    adjust_args(p, (Func*)f);
    adjust_lvars(p, (Func*)f);

    // move arguments and initialize locals
    pr_move(p, pr_bp(p), pr_sp(p)-fn_argc(f), fn_argc(f));
    pr_initf(p, pr_bp(p)+fn_argc(f), fn_lvarc(f), NOTHING);

    // install registers
    p->fs = fn_fsize(f);
    p->pf = f;
    p->sp = pr_bp(p)+pr_fs(p);
    p->fp = pr_sp(p);

  } else { // tail recursive call (frame size can be safely assumed)
    // move arguments to bp and initialize other locals
    pr_move(p, pr_bp(p), p->sp-fn_argc(f), fn_argc(f));
    pr_initf(p, pr_bp(p)+fn_argc(f), NOTHING, fn_lvarc(f));

    // install registers
    p->fs = fn_fsize(f);
    p->sp = pr_bp(p) + pr_fs(p); // discard any remaining temporaries
  }
}

static void install_proto(Proc* p, Proto* f, int t) {
  // NB: arguments to call already on stack and validated

  if ( t == -1 ) { // not a tail call
    pr_shrinkf(p, f->argc);           // remove arguments to f from calling frame
    pr_reserve(p, f->lvarc, NOTHING); // reserve space for other locals
    pr_savef(p);                      // save current frame

    // install registers
    p->fs = fn_fsize(f);
    p->cl = f;
    p->ip = fn_ip(f);
    p->bp = pr_sp(p)-pr_fs(p);
    p->fp = pr_sp(p);

  } else if ( t == 0 ) { // general tail call (frame may need to be resized)
    // adjust size
    adjust_args(p, (Func*)f);
    adjust_lvars(p, (Func*)f);

    // move arguments and initialize locals
    pr_move(p, pr_bp(p), pr_sp(p)-fn_argc(f), fn_argc(f));
    pr_initf(p, pr_bp(p)+fn_argc(f), fn_lvarc(f), NOTHING);

    // install registers
    p->fs = fn_fsize(f);
    p->cl = f;
    p->ip = fn_ip(f);
    p->sp = pr_bp(p)+pr_fs(p);
    p->fp = pr_sp(p);

  } else { // tail recursive call (frame size can be safely assumed)
    // move arguments to bp and initialize other locals
    pr_move(p, pr_bp(p), p->sp-fn_argc(f), fn_argc(f));
    pr_initf(p, pr_bp(p)+fn_argc(f), NOTHING, fn_lvarc(f));

    // discard remaining temporaries and install registers
    p->fs = fn_fsize(f);
    p->ip = fn_ip(f);
    p->sp = pr_bp(p) + pr_fs(p); // discard any remaining temporaries
  }
}

static void install_hndl(Proc* p, Proto* b, Proto* h) {
  // install the hndl body
  install_proto(p, b, -1);

  // save body and current frame pointer
  pr_fpush(p, tag(h));
  pr_fpush(p, tag(p->fp));

  // set handle pointer
  p->hp = pr_sp(p);
}

static bool has_hndl(Proc* p) {
  Val* hp = pr_hp(p);
  bool o = false;
  Proto* h;

  while ( o == false && hp != NULL ) {
    h = as_proto(hp[-2]);
    o = h->hndl_h;
    h = as_ptr(hp[-1]);
  }

  return o;
}

static bool has_catch(Proc* p) {
  Val* hp = pr_hp(p);
  bool o  = false;
  Proto* h;

  while ( o == false && hp != NULL ) {
    h = as_proto(hp[-2]);
    o = h->catch_h;
    h = as_ptr(hp[-1]);
  }

  return o;
}

static void pop_hndl(Proc* p);

static bool restore_hndl(Proc* p, Proto** h, Cntl** k) {
  bool o = has_hndl(p);

  if ( o ) {
    bool d = false;
    Val* f;

    while ( !d ) {
      f  = pr_hp(p);
      *h = as_proto(f[-2]);
      *k = mk_cntl(p, *k);
      d  = (*h)->hndl_h;
      pop_hndl(p);
    }
  }

  return o;
}

static void  apply_raise(Proc* p, Proto* h, Cntl* k, Sym* o, Val a);
static void  install_catch(Proc* p, Proto* b, Proto* h);
static bool  restore_catch(Proc* p, Proto** h);
static void  apply_throw(Proc* p, Proto* h, Sym* e, Str* m, Val b);

static inline int do_fetch32(Proc* p) {
  int b;

  memcpy(&b, p->ip, sizeof(int));
  p->ip += 2;

  return b;
}

/* Internal */
Error rl_exec_at(Proc* p, Val* r, Label e) {

// local macros
#define jmp_nx()             goto *next
#define jmp_err(p)           goto *labels[p->err]
#define jmp(l)               goto l
#define jmp_lbl(l)           goto *labels[l]

#define chk_pop(p)                              \
  if ( unlikely(pr_chk_pop(p, E_RUN)) )         \
    jmp_err(p)

#define chk_push(p)                             \
  if ( unlikely(pr_chk_push(p, E_RUN)) )        \
    jmp_err(p)

#define chk_pushn(p, n)                                         \
  if ( unlikely(pr_chk_pushn(p, E_EVAL, n)) )                   \
    jmp_err(p)
  
#define chk_stkn(p, n)                                          \
  if ( unlikely(pr_chk_stkn(p, E_RUN, n)) )                     \
    jmp_err(p)

#define chk_def(p, r)        unlikely(pr_chk_def(p, E_EVAL, r))
#define chk_refv(p, r)       unlikely(pr_chk_refv(p, E_EVAL, r))
#define chk_reft(p, r, g)    unlikely(pr_chk_reft(p, E_EVAL, r, g))
#define chk_type(p, x, g, s) unlikely(pr_chk_type(p, E_EVAL, x, g, s))

#define push_nx(p, l)                           \
  do {                                          \
    if ( reg(nx) < L_READY ) {                  \
      chk_push(p);                              \
      pr_push(p, tag(p->nx));                   \
    }                                           \
    mov(reg(nx), l);                            \
    mov(next, labl(l));                         \
  } while (false)

#define pop_nx(p)                               \
  do {                                          \
    mov(reg(nx), as_small(pr_pop(p)));          \
    mov(next, labl(reg(nx)));                   \
  } while (false)


#define rx_push(p, rx)                          \
  pr_push(p, tag(p->rx))

#define rx_pop(p, rx)                           \
  p->rx = untag(pr_pop(p))

#define rx_fpush(p, rx)                         \
  pr_fpush(p, tag(p->rx))

#define rx_fpop(p, rx)                          \
  p->rx = untag(pr_fpop(p))

  static void* labels[] = {
    /* error labels */
    [E_OKAY]     = &&e_okay,
    [E_SYS]      = &&e_sys,
    [E_RUN]      = &&e_run,
    [E_READ]     = &&e_read,
    [E_COMP]     = &&e_comp,
    [E_STX]      = &&e_stx,
    [E_GENFN]    = &&e_genfn,
    [E_EVAL]     = &&e_eval,
    [E_USER]     = &&e_user,

    /* opcode labels */
    // miscellaneous
    [O_NOOP]     = &&o_noop,

    // stack manipulation
    [O_POP]      = &&o_pop,
    [O_DUP]      = &&o_dup,

    // constant loads
    [O_LD_NUL]   = &&o_ld_nul,
    [O_LD_TRUE]  = &&o_ld_true,
    [O_LD_FALSE] = &&o_ld_false,
    [O_LD_ZERO]  = &&o_ld_zero,
    [O_LD_ONE]   = &&o_ld_one,

    // register loads
    [O_LD_FUN]   = &&o_ld_fun,
    [O_LD_ENV]   = &&o_ld_env,

    // inlined loads
    [O_LD_S16]   = &&o_ld_s16,
    [O_LD_S32]   = &&o_ld_s32,
    [O_LD_G16]   = &&o_ld_g16,
    [O_LD_G32]   = &&o_ld_g32,

    // standard loads
    [O_LD_VAL]   = &&o_ld_val,
    [O_LD_STK]   = &&o_ld_stk,
    [O_PUT_STK]  = &&o_put_stk,
    [O_LD_UPV]   = &&o_ld_upv,
    [O_PUT_UPV]  = &&o_put_upv,
    [O_LD_CNS]   = &&o_ld_cns,
    [O_PUT_CNS]  = &&o_put_cns,
    [O_LD_QNS]   = &&o_ld_qns,
    [O_PUT_QNS]  = &&o_put_qns,

    // nonlocal control constructs
    [O_HNDL]     = &&o_hndl,
    [O_RAISE1]   = &&o_raise1,
    [O_RAISE2]   = &&o_raise2,
    [O_RAISE3]   = &&o_raise3,
    [O_CATCH]    = &&o_catch,
    [O_THROW1]   = &&o_throw1,
    [O_THROW2]   = &&o_throw2,
    [O_THROW3]   = &&o_throw3,

    // branch instructions
    [O_JMP]      = &&o_jmp,
    [O_JMPT]     = &&o_jmpt,
    [O_JMPF]     = &&o_jmpf,

    // closure/upvalue instructions
    [O_CLOSURE]  = &&o_closure,
    [O_CAPTURE]  = &&o_capture,

    // exit/return/cleanup instructions
    [O_RETURN]   = &&o_return,
    [O_EXIT]     = &&o_exit,

    /* primfn labels */
    [F_TYPEOF]   = &&f_typeof,
    [F_HASH]     = &&f_hash,
    [F_SAME]     = &&f_same,

    /* read labels */

    /* compile labels */

    /* general labels */
    [L_NEXT]     = &&l_next,
  };

  int tf;                // tail call flag
  int ax, ay, az;        // temporary registers for bytecode arguments
  Val vx, vy, vz;        // general temporary registers
  UpVal* ux;
  Val* vxs;
  Ref* rx;
  Func* fx;
  Cntl* kx;             
  Proto* cx, * bx, * hx; // closure, body, handler
  Env* ex;
  Type* tx;
  Str* mx;
  Sym* ox;
  Label o;
  void *next;

  push_nx(p, e);
  jmp_nx();

  /* error labels */
  // not an error
 e_okay:
  jmp_nx();

  // unrecoverable errors
 e_sys:
 e_run:
 e_norecover:
  pr_prn_err(p);
  mov(*r, NOTHING);
  jmp(end);

 e_read:
 e_comp:
 e_stx:
 e_genfn:
 e_eval:
 e_user:
  if ( restore_catch(p, &hx) )
    jmp(e_norecover);

  pr_recover(p, &ox, &mx, &vx);   // clear error
  apply_throw(p, hx, ox, mx, vx); // 
  jmp_nx();

 l_next:
  mov(o, fetch16(p));             // get next opcode
  jmp_lbl(o);                     // dispatch to label

  /* opcode labels */
 o_noop:
  jmp_nx();

 o_pop:
  chk_pop(p);
  pr_fpop(p);
  jmp_nx();

 o_dup:
  chk_stkn(p, 1);
  chk_push(p);
  pr_dup(p);
  jmp_nx();

 o_ld_nul:
  chk_push(p);
  pr_fpush(p, NUL);
  jmp_nx();

 o_ld_true:
  chk_push(p);
  pr_fpush(p, TRUE);
  jmp_nx();

 o_ld_false:
  chk_push(p);
  pr_fpush(p, FALSE);
  jmp_nx();

 o_ld_zero:
  chk_push(p);
  pr_fpush(p, ZERO);
  jmp_nx();

 o_ld_one:
  chk_push(p);
  pr_fpush(p, ONE);
  jmp_nx();

 o_ld_fun:
  chk_push(p);
  rx_fpush(p, fn);
  jmp_nx();

 o_ld_env:
  chk_push(p);
  rx_fpush(p, nv);
  jmp_nx();

 o_ld_s16:
  chk_push(p);
  fetch16(ax, p);
  pr_fpush(p, tag(ax));
  jmp_nx();

 o_ld_s32:
  chk_push(p);
  fetch32(ax, p);
  pr_fpush(p, tag(ax));
  jmp_nx();

 o_ld_g16:
  chk_push(p);
  fetch16(ax, p);
  pr_fpush(p, tag_glyph(ax));
  jmp_nx();

 o_ld_g32:
  chk_push(p);
  fetch32(ax, p);
  pr_fpush(p, tag_glyph(ax));
  jmp_nx();

 o_ld_val:
  chk_push(p);
  fetch16(ax, p);
  val(vx, ax);
  pr_fpush(p, vx);
  jmp_nx();

 o_ld_stk:
  chk_push(p);
  fetch16(ax, p);
  local(vx, ax);
  pr_fpush(p, vx);
  jmp_nx();

 o_put_stk:
  fetch16(ax, p);
  mov(vx, pr_fpop(p));
  mov(local(ax), vx);

  jmp_nx();

 o_ld_upv:
  chk_push(p);
  fetch16(ax, p);
  upval(ux, ax);
  mov(vxs, dr_upv(ux));
  mov(vx, *vxs);
  pr_fpush(p, vx);
  jmp_nx();

 o_put_upv:
  ax   = fetch16(p);
  vx   = pr_fpop(p);
  ux   = pr_upvs(p)[ax];
  vxs  = dr_upv(ux);
  *vxs = vx;

  jmp_nx();

 o_ld_cns:
  ay = -1;
  ax = fetch16(p);

  goto do_get_ns_ref;

 o_put_cns:
  ay = -1;
  ax = fetch16(p);

  goto do_put_ns_ref;

 o_ld_qns:
  ay = fetch16(p);
  ax = fetch16(p);

  goto do_get_ns_ref;

 o_put_qns:
  ay = fetch16(p);
  ax = fetch16(p);

  goto do_put_ns_ref;

 do_get_ns_ref:
  if ( chk_push(p) )
    goto error;

  rx = get_ns_ref(p, ay, ax);

  if ( chk_def(p, rx) ) // check that reference is defined
    goto error;

  vx = rx->val;

  pr_fpush(p, vx);

  jmp_nx();

 do_put_ns_ref:
  rx = get_ns_ref(p, ay, ax);

  if ( chk_def(p, rx) ) // check that reference is defined
    goto error;

  if ( chk_refv(p, rx) ) // check that reference can be assigned
    goto error;

  if ( chk_reft(p, rx, vx) ) // check that assignment has correct type
    goto error;

  rx->val = vx;

  if ( rx->final ) { // infer type
    rx->tag = type_of(vx);
    set_meta(rx, ":tag", tag(rx->tag));
  }

  jmp_nx();

 o_hndl:
  vx = pr_fpop(p); // handler
  vy = pr_fpop(p); // body

  if ( chk_type(p, &ProtoType, vx, &hx) )
    goto error;

  if ( chk_type(p, &ProtoType, vy, &bx) )
    goto error;

  if ( chk_pushn(p, fn_fsize(bx)+2) )
    goto error;

  install_hndl(p, bx, hx);

  jmp_nx();

 o_raise1:
  vx = NOTHING;    // operation continuation (ignored)
  vy = pr_fpop(p); // operation label (must be Sym)
  vz = NOTHING;    // operation argument (ignored)

  goto do_raise;

 o_raise2:
  vx = NOTHING;
  vy = pr_fpop(p);
  vz = pr_fpop(p);

  if ( is_cntl(vy) ) {
    vx = vy;
    vy = vz;
    vz = NOTHING;
  }

  goto do_raise;
  
 o_raise3:
  vx = pr_fpop(p); // operation continuation (must be Cntl)
  vy = pr_fpop(p); // operation label (must be Sym)
  vz = pr_fpop(p); // operation argument (can be anything)

  goto do_raise;

 do_raise:
  if ( vx == NOTHING )
    kx = NULL;

  else if ( chk_type(p, &CntlType, vx, &kx) )
    goto error;

  if ( chk_type(p, &SymType, vy, &ox) )
    goto error;

  if ( restore_hndl(p, &hx, &kx) )
    goto error;
  
 o_catch:
  
 o_throw1:
 o_throw2:
 o_throw3:
  
 o_jmp:
  ax   = fetch16(p);
  p->fp += ax;

  jmp_nx();

 o_jmpt:
  ax   = fetch16(p);
  vx   = pr_fpop(p);

  if ( truthy(vx) )
    p->fp += ax;

  jmp_nx();

 o_jmpf:
  ax = fetch16(p);
  vx = pr_fpop(p);

  if ( falsey(vx) )
    p->fp += ax;

  jmp_nx();

 o_closure:
  ax        = fetch16(p);            // number of uxalues to be initialized
  cx        = as_proto(p->sp[-1]); // fetch prototype of new closure
  cx        = bind_proto(cx);       // create bound prototype (aka closure)
  p->sp[-1] = tag(cx);              // return to stack in case allocating uxalues triggers gc

  // initialize uxalues
  for ( int i=0; i < ax; i++ ) {
    fetch16(ay, p); // local?
    fetch16(az, p); // offset

    if ( ay )
      cl_upvs(cx)[i] = open_upv(&p->upvs, &p->bp[az]);

    else
      cl_upvs(cx)[i] = pr_upvs(p)[az];
  }

  jmp_nx();

 o_capture:
  close_upvs(&p->upvs, pr_bp(p));
  jmp_nx();

 o_call0:
 o_call1:
 o_call2:
 o_calln:
 o_tcall0:
 o_tcall1:
 o_tcall2:
 o_tcalln:
 o_scall0:
 o_scall1:
 o_scall2:
 o_scalln:
 o_apply1:
 o_apply2:
 o_applyn:
 o_tapply1:
 o_tapply2:
 o_tapplyn:
 o_sapply1:
 o_sapply2:
 o_sapplyn:
  
 o_return:
  vx = pr_sp(p)[-1]; // return value
  pr_restoref(p);    // restore caller
  pr_fpush(p, vx);   // push return value
  jmp_nx();   // jump to caller

 o_exit:
  goto end;

 f_typeof:
  /* Get the data type of TOS */
  vx        = p->sp[-1];
  tx        = type_of(vx);
  p->sp[-1] = tag(tx);

  jmp_nx();

 f_hash:
  vx        = p->sp[-1];
  vy        = rl_hash(vx, false);
  p->sp[-1] = tag_arity(vy);

  jmp_nx();

 f_same:
  vx        = pr_fpop(p);
  vy        = p->sp[-1];
  vz        = vx == vy ? TRUE : FALSE;
  p->sp[-1] = vz;

  jmp_nx();

 end:
  if ( p->err == E_OKAY )
    mov(*r, pr_pop(p));

  return p->err;

#undef chk_pop
#undef chk_push
#undef chk_pushn
#undef chk_stkn
#undef chk_def
#undef chk_refv
#undef chk_reft
#undef chk_type
#undef fetch
}

/* External APIs */
Error rl_exec(Proc* p, Proto* c, Val* r) {
  reset_pr(p);
  install_proto(p, c, -1);
  Error o;

  if ( (o=rl_exec_at(p, r, L_NEXT)) )
    *r = NOTHING;

  reset_pr(p);

  return o;
}
