#include <string.h>

#include "lang/exec.h"

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

static void  install_proto(Proc* p, Proto* f, int t) {
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
#define chk_pop(p)           unlikely(pr_chk_pop(p, RUN_ERR))
#define chk_push(p)          unlikely(pr_chk_push(p, RUN_ERR))
#define chk_pushn(p, n)      unlikely(pr_chk_pushn(p, EVAL_ERR, n))
#define chk_stkn(p, n)       unlikely(pr_chk_stkn(p, RUN_ERR, n))
#define chk_def(p, r)        unlikely(pr_chk_def(p, EVAL_ERR, r))
#define chk_refv(p, r)       unlikely(pr_chk_refv(p, EVAL_ERR, r))
#define chk_reft(p, r, g)    unlikely(pr_chk_reft(p, EVAL_ERR, r, g))
#define chk_type(p, x, g, s) unlikely(pr_chk_type(p, EVAL_ERR, x, g, s))

#define fetch(p)             *p->ip++

  
  static void* labels[] = {
    // miscellaneous
    [OP_NOOP]     = &&op_noop,
    [OP_ENTER]    = &&op_enter,

    // stack manipulation
    [OP_POP]      = &&op_pop,
    [OP_DUP]      = &&op_dup,

    // constant loads
    [OP_LD_NUL]   = &&op_ld_nul,
    [OP_LD_TRUE]  = &&op_ld_true,
    [OP_LD_FALSE] = &&op_ld_false,
    [OP_LD_ZERO]  = &&op_ld_zero,
    [OP_LD_ONE]   = &&op_ld_one,

    // register loads
    [OP_LD_FUN]   = &&op_ld_fun,
    [OP_LD_ENV]   = &&op_ld_env,

    // inlined loads
    [OP_LD_S16]   = &&op_ld_s16,
    [OP_LD_S32]   = &&op_ld_s32,
    [OP_LD_G16]   = &&op_ld_g16,
    [OP_LD_G32]   = &&op_ld_g32,

    // standard loads
    [OP_LD_VAL]   = &&op_ld_val,
    [OP_LD_STK]   = &&op_ld_stk,
    [OP_PUT_STK]  = &&op_put_stk,
    [OP_LD_UPV]   = &&op_ld_upv,
    [OP_PUT_UPV]  = &&op_put_upv,
    [OP_LD_CNS]   = &&op_ld_cns,
    [OP_PUT_CNS]  = &&op_put_cns,
    [OP_LD_QNS]   = &&op_ld_qns,
    [OP_PUT_QNS]  = &&op_put_qns,

    // nonlocal control constructs
    [OP_HNDL]     = &&op_hndl,
    [OP_RAISE]    = &&op_raise,
    [OP_CATCH]    = &&op_catch,
    [OP_THROW]    = &&op_throw,

    // branch instructions
    [OP_JMP]      = &&op_jmp,
    [OP_JMPT]     = &&op_jmpt,
    [OP_JMPF]     = &&op_jmpf,

    // closure/upvalue instructions
    [OP_CLOSURE]  = &&op_closure,
    [OP_CAPTURE]  = &&op_capture,

    // exit/return/cleanup instructions
    [OP_RETURN]   = &&op_return,
    [OP_EXIT]     = &&op_exit,
  };

  int argx, argy, argz;
  Val tmpx, tmpy;
  UpVal* upv;
  Val* spc;
  Ref* ref;
  Func* f;
  Cntl* k;
  Proto* c, * b, * h;
  Str* msg;
  Sym* op;
  Label lb;

  goto *labels[e];

 next:
  lb = fetch(p);
  goto *labels[lb];

 error:
  // not recoverable
  if ( p->err < READ_ERR || !restore_catch(p, &h) ) {
    pr_prn_err(p);           // print current error
    *r = NOTHING;            // set return
    goto end;               // escape

  } else {
    pr_recover(p, &op, &msg, &tmpx);     // clear error
    apply_throw(p, h, op, msg, tmpx);    // apply handler
    goto next;                           // jump to handler
  }

 end:
  if ( p->err == OKAY )
    *r = pr_pop(p);

  return p->err;

 op_enter:
 op_noop:
  goto next;

 op_pop:
  if ( chk_pop(p) )
    goto error;

  pr_fpop(p);

  goto next;

 op_dup:
  if ( chk_stkn(p, 1) )
    goto error;

  if ( chk_push(p) )
    goto error;
  
  pr_dup(p);

  goto next;

 op_ld_nul:
  if ( chk_push(p) )
    goto error;

  pr_fpush(p, NUL);

  goto next;

 op_ld_true:
  if ( chk_push(p) )
    goto error;

  pr_fpush(p, TRUE);

  goto next;

 op_ld_false:
  if ( chk_push(p) )
    goto error;

  pr_fpush(p, FALSE);

  goto next;

 op_ld_zero:
  if ( chk_push(p) )
    goto error;

  pr_fpush(p, ZERO);

  goto next;

 op_ld_one:
  if ( chk_push(p) )
    goto error;

  pr_fpush(p, ONE);

  goto next;

 op_ld_fun:
  if ( chk_push(p) )
    goto error;

  pr_fpush(p, tag(p->fn));

  goto next;

 op_ld_env:
  if ( chk_push(p) )
    goto error;

  pr_fpush(p, tag(pr_env(p)));

  goto next;

 op_ld_s16:
  if ( chk_push(p) )
    goto error;
  
  argx = fetch(p);

  pr_fpush(p, tag(argx));

  goto next;

 op_ld_s32:
  if ( chk_push(p) )
    goto error;

  argx = fetch32(p);

  pr_fpush(p, tag(argx));

  goto next;

 op_ld_g16:
  if ( chk_push(p) )
    goto error;

  argx = fetch(p);

  pr_fpush(p, tag_glyph(argx));

  goto next;

 op_ld_g32:
  if ( chk_push(p) )
    goto error;

  argx = fetch32(p);

  pr_fpush(p, tag_glyph(argx));

  goto next;

 op_ld_val:
  if ( chk_push(p) )
    goto error;

  argx = fetch(p);
  tmpx = pr_vals(p)[argx];

  pr_fpush(p, tmpx);

  goto next;

 op_ld_stk:
  if ( chk_push(p) )
    goto error;

  argx = fetch(p);
  tmpx = p->bp[argx];

  pr_fpush(p, tmpx);

  goto next;

 op_put_stk:
  argx        = fetch(p);
  tmpx        = pr_fpop(p);
  p->bp[argx] = tmpx;

  goto next;

 op_ld_upv:
  if ( chk_push(p) )
    goto error;
  
  argx = fetch(p);
  upv  = pr_upvs(p)[argx];
  spc  = dr_upv(upv);
  tmpx = *spc;

  pr_fpush(p, tmpx);

  goto next;

 op_put_upv:
  argx = fetch(p);
  tmpx = pr_fpop(p);
  upv  = pr_upvs(p)[argx];
  spc  = dr_upv(upv);
  *spc = tmpx;

  goto next;

 op_ld_cns:
  argy = -1;
  argx = fetch(p);

  goto do_get_ns_ref;

 op_put_cns:
  argy = -1;
  argx = fetch(p);

  goto do_put_ns_ref;

 op_ld_qns:
  argy = fetch(p);
  argx = fetch(p);

  goto do_get_ns_ref;

 op_put_qns:
  argy = fetch(p);
  argx = fetch(p);

  goto do_put_ns_ref;

 do_get_ns_ref:
  if ( chk_push(p) )
    goto error;

  ref = get_ns_ref(p, argy, argx);

  if ( chk_def(p, ref) ) // check that reference is defined
    goto error;

  tmpx = ref->val;

  pr_fpush(p, tmpx);

  goto next;

 do_put_ns_ref:
  ref = get_ns_ref(p, argy, argx);

  if ( chk_def(p, ref) ) // check that reference is defined
    goto error;

  if ( chk_refv(p, ref) ) // check that reference can be assigned
    goto error;

  if ( chk_reft(p, ref, tmpx) ) // check that assignment has correct type
    goto error;

  ref->val = tmpx;

  if ( ref->final ) { // infer type
    ref->tag = type_of(tmpx);
    set_meta(ref, ":tag", tag(ref->tag));
  }

  goto next;

 op_hndl:
  tmpx = pr_fpop(p); // handler
  tmpy = pr_fpop(p); // body

  if ( chk_type(p, &ProtoType, tmpx, &h) )
    goto error;

  if ( chk_type(p, &ProtoType, tmpy, &c) )
    goto error;

  if ( chk_pushn(p, fn_fsize(c)+2) )
    goto error;

  install_hndl(p, c, h);

  goto next;

 op_raise:
  tmpx = pr_fpop(p); // arguments
  tmpy = pr_fpop(p); // 
    
 op_catch:
  
 op_throw:

 op_jmp:
  argx   = fetch(p);
  p->fp += argx;

  goto next;

 op_jmpt:
  argx   = fetch(p);
  tmpx   = pr_fpop(p);

  if ( truthy(tmpx) )
    p->fp += argx;

  goto next;

 op_jmpf:
  argx = fetch(p);
  tmpx = pr_fpop(p);

  if ( falsey(tmpx) )
    p->fp += argx;

  goto next;

 op_closure:
  argx      = fetch(p);            // number of upvalues to be initialized
  c         = as_proto(p->sp[-1]); // fetch prototype of new closure
  c         = bind_proto(c);       // create bound prototype (aka closure)
  p->sp[-1] = tag(c);              // return to stack in case allocating upvalues triggers gc

  // initialize upvalues
  for ( int i=0; i<argx; i++ ) {
    argy = fetch(p); // local?
    argz = fetch(p); // offset

    if ( argy )
      cl_upvs(c)[i] = open_upv(&p->upvs, &p->bp[argz]);

    else
      cl_upvs(c)[i] = pr_upvs(p)[argz];
  }

  goto next;

 op_capture:
  close_upvs(&p->upvs, pr_bp(p));
  goto next;
  
 op_return:
  tmpx = pr_sp(p)[-1]; // return value
  pr_restoref(p);      // restore caller
  pr_fpush(p, tmpx);   // push return value
  goto next;           // jump to caller

 op_exit:
  goto end;

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

  if ( (o=rl_exec_at(p, r, OP_ENTER)) )
    *r = NOTHING;

  reset_pr(p);

  return o;
}
