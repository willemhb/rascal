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

static inline int fetch32(Proc* p) {
  int buf;

  // can't guarantee alignment so doing this with memcpy
  memcpy(&buf, p->ip, sizeof(int));

  p->ip += 2;

  return buf;
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

/* Internal */
Error rl_exec_at(Proc* p, Val* r, Label e) {

  // local macros
#define next(p)              *labels[p->nx]
#define chk_pop(p)           unlikely(pr_chk_pop(p, E_RUN))
#define chk_push(p)          unlikely(pr_chk_push(p, E_RUN))
#define chk_pushn(p, n)      unlikely(pr_chk_pushn(p, E_EVAL, n))
#define chk_stkn(p, n)       unlikely(pr_chk_stkn(p, E_RUN, n))
#define chk_def(p, r)        unlikely(pr_chk_def(p, E_EVAL, r))
#define chk_refv(p, r)       unlikely(pr_chk_refv(p, E_EVAL, r))
#define chk_reft(p, r, g)    unlikely(pr_chk_reft(p, E_EVAL, r, g))
#define chk_type(p, x, g, s) unlikely(pr_chk_type(p, E_EVAL, x, g, s))

#define fetch(p)             *p->ip++

  static void* labels[] = {
    /* error labels */
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
  Label o,n = e;
  void* next = labels[n];

  goto *next;

 l_next:
  o    = fetch(p);
  goto *labels[o];

 error:
  // not recoverable
  if ( p->err < E_READ || !restore_catch(p, &hx) ) {
    pr_prn_err(p);           // print current error
    *r = NOTHING;            // set return
    goto end;                // escape

  } else {
    pr_recover(p, &ox, &mx, &vx);   // clear error
    apply_throw(p, hx, ox, mx, vx); // apply handler
    goto next(p);
  }

 end:
  if ( p->err == E_OKAY )
    *r = pr_pop(p);

  return p->err;

  /* opcode labels */
 o_noop:
  goto *labels[n];

 o_pop:
  if ( chk_pop(p) )
    goto error;

  pr_fpop(p);

  goto *labels[n];

 o_dup:
  if ( chk_stkn(p, 1) )
    goto error;

  if ( chk_push(p) )
    goto error;
  
  pr_dup(p);

  goto 

 o_ld_nul:
  if ( chk_push(p) )
    goto error;

  pr_fpush(p, NUL);

  goto next(p);

 o_ld_true:
  if ( chk_push(p) )
    goto error;

  pr_fpush(p, TRUE);

  goto *labels[n];

 o_ld_false:
  if ( chk_push(p) )
    goto error;

  pr_fpush(p, FALSE);

  goto *labels[n];

 o_ld_zero:
  if ( chk_push(p) )
    goto error;

  pr_fpush(p, ZERO);

  goto *labels[n];

 o_ld_one:
  if ( chk_push(p) )
    goto error;

  pr_fpush(p, ONE);

  goto *labels[n];

 o_ld_fun:
  if ( chk_push(p) )
    goto error;

  pr_fpush(p, tag(p->fn));

  goto *labels[n];

 o_ld_env:
  if ( chk_push(p) )
    goto error;

  pr_fpush(p, tag(pr_env(p)));

  goto *labels[n];

 o_ld_s16:
  if ( chk_push(p) )
    goto error;
  
  ax = fetch(p);

  pr_fpush(p, tag(ax));

  goto *labels[n];

 o_ld_s32:
  if ( chk_push(p) )
    goto error;

  ax = fetch32(p);

  pr_fpush(p, tag(ax));

  goto *labels[n];

 o_ld_g16:
  if ( chk_push(p) )
    goto error;

  ax = fetch(p);

  pr_fpush(p, tag_glyph(ax));

  goto *labels[n];

 o_ld_g32:
  if ( chk_push(p) )
    goto error;

  ax = fetch32(p);

  pr_fpush(p, tag_glyph(ax));

  goto *labels[n];

 o_ld_val:
  if ( chk_push(p) )
    goto error;

  ax = fetch(p);
  vx = pr_vals(p)[ax];

  pr_fpush(p, vx);

  goto *labels[n];

 o_ld_stk:
  if ( chk_push(p) )
    goto error;

  ax = fetch(p);
  vx = p->bp[ax];

  pr_fpush(p, vx);

  goto *labels[n];

 o_put_stk:
  ax        = fetch(p);
  vx        = pr_fpop(p);
  p->bp[ax] = vx;

  goto *labels[n];

 o_ld_upv:
  if ( chk_push(p) )
    goto error;

  ax  = fetch(p);
  ux  = pr_upvs(p)[ax];
  vxs = dr_upv(ux);
  vx  = *vxs;

  pr_fpush(p, vx);

  goto *labels[n];

 o_put_upv:
  ax   = fetch(p);
  vx   = pr_fpop(p);
  ux   = pr_upvs(p)[ax];
  vxs  = dr_upv(ux);
  *vxs = vx;

  goto *labels[n];

 o_ld_cns:
  ay = -1;
  ax = fetch(p);

  goto do_get_ns_ref;

 o_put_cns:
  ay = -1;
  ax = fetch(p);

  goto do_put_ns_ref;

 o_ld_qns:
  ay = fetch(p);
  ax = fetch(p);

  goto do_get_ns_ref;

 o_put_qns:
  ay = fetch(p);
  ax = fetch(p);

  goto do_put_ns_ref;

 do_get_ns_ref:
  if ( chk_push(p) )
    goto error;

  rx = get_ns_ref(p, ay, ax);

  if ( chk_def(p, rx) ) // check that reference is defined
    goto error;

  vx = rx->val;

  pr_fpush(p, vx);

  goto *labels[n];

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

  goto *labels[n];

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

  goto *labels[n];

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
  ax   = fetch(p);
  p->fp += ax;

  goto *labels[n];

 o_jmpt:
  ax   = fetch(p);
  vx   = pr_fpop(p);

  if ( truthy(vx) )
    p->fp += ax;

  goto *labels[n];

 o_jmpf:
  ax = fetch(p);
  vx = pr_fpop(p);

  if ( falsey(vx) )
    p->fp += ax;

  goto *labels[n];

 o_closure:
  ax        = fetch(p);            // number of uxalues to be initialized
  cx        = as_proto(p->sp[-1]); // fetch prototype of new closure
  cx        = bind_proto(cx);       // create bound prototype (aka closure)
  p->sp[-1] = tag(cx);              // return to stack in case allocating uxalues triggers gc

  // initialize uxalues
  for ( int i=0; i < ax; i++ ) {
    ay = fetch(p); // local?
    az = fetch(p); // offset

    if ( ay )
      cl_upvs(cx)[i] = open_upv(&p->upvs, &p->bp[az]);

    else
      cl_upvs(cx)[i] = pr_upvs(p)[az];
  }

  goto *labels[n];

 o_capture:
  close_upvs(&p->upvs, pr_bp(p));
  goto *labels[n];

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
  goto *labels[n];   // jump to caller

 o_exit:
  goto end;

 f_typeof:
  /* Get the data type of TOS */
  vx        = p->sp[-1];
  tx        = type_of(vx);
  p->sp[-1] = tag(tx);

  goto *labels[n];

 f_hash:
  vx        = p->sp[-1];
  vy        = rl_hash(vx, false);
  p->sp[-1] = tag_arity(vy);

  goto *labels[n];

 f_same:
  vx        = pr_fpop(p);
  vy        = p->sp[-1];
  vz        = vx == vy ? TRUE : FALSE;
  p->sp[-1] = vz;

  goto *labels[n];

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
