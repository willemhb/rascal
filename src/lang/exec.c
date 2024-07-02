#include <string.h>

#include "lang/exec.h"

#include "val/function.h"
#include "val/environ.h"
#include "val/text.h"

#include "vm/state.h"
#include "vm/environ.h"

#include "util/number.h"

/* Internal APIs */
static inline bool truthy(Val x) {
  return x != FALSE && x != NUL;
}

static inline bool falsey(Val x) {
  return x == FALSE || x == NUL;
}

static inline int fetch32(short** ip) {
  int buf;

  memcpy(&buf, *ip, sizeof(int));

  *ip += 2;

  return buf;
}

static inline UpVal** cl_upvs(RlProc* p) {
  return p->cl->envt->upvals;
}

static inline Val* cl_vals(RlProc* p) {
  return p->cl->vals->tail;
}

static Ref* get_ns_ref(RlProc* p, int yoff, int xoff) {
  Env* nv;
  Ref* o = NULL;

  if ( yoff == -1 ) {
    nv = rlp_env(p)->ns;

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

static void save_frame(RlProc* p) {
  if ( is_proto(p->fn) )
    rlp_pushn(p, 3, tag(p->fn), tag(p->ip), tag(p->fs));

  else
    rlp_pushn(p, 3, tag(p->fn), tag(p->nv), tag(p->fs));
}

static void  install_primfn(RlProc* p, PrimFn* f, int t);
static void  install_proto(RlProc* p, Proto* f, int t);
static void  install_catch(RlProc* p, Proto* b, Proto* h);
static void  restore_catch(RlProc* p, Proto** h);
static void  apply_throw(RlProc* p, Proto* h, Sym* e, Str* m, Val b);
static void  install_hndl(RlProc* p, Proto* b, Proto* h);
static void  restore_hndl(RlProc* p, Proto** h);
static void  apply_raise(RlProc* p, Proto* h, Cntl* k, Sym* o, Val a);

static inline size_t fpush(RlProc* p, Val x) {
  p->fs++;

  return rlp_push(p, x);
}

static inline size_t fpop(RlProc* p) {
  p->fs--;

  return rlp_pop(p);
}

static inline size_t fdup(RlProc* p) {
  p->fs++;

  return rlp_dup(p);
}

/* Internal */
Error rl_exec_at(RlProc* p, Val* r, Label e) {
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

    // 
  };

  int argx, argy, argz;
  Val tmpx, tmpy;
  UpVal* upv;
  Val* spc;
  Ref* ref;
  Func* f;
  Proto* c, * b, * h;
  Str* errm;
  Sym* errk;

  goto *labels[e];

 next:
  p->lb = *p->ip++;
  goto *labels[p->lb];

 error:
  if ( p->err < READ_ERR || p->cp == 0 ) { // not recoverable
    rlp_prn_err(p);
    *r = NOTHING;
    goto exit;
  } else {
    rlp_recover(p, &errk, &errm, &tmpx);
    argx = tmpx == NOTHING ? 2 : 3;
    goto recover;
  }

 recover:
  // restore catch frame (discards everything between current TOS and current cp)
  restore_catch(p, &h);

  // apply catch handler
  apply_throw(p, h, errk, errm, tmpx);

  // jump to handler
  goto next;
  
 exit:
  if ( p->err == OKAY )
    *r = fpop(p);

  return p->err;

 op_enter:
 op_noop:
  goto next;

 op_pop:
  if ( unlikely(rlp_chk_pop(p, RUN_ERR)) )
    goto error;

  fpop(p);

  goto next;

 op_dup:
  if ( unlikely(rlp_chk_push(p, RUN_ERR)) )
    goto error;
  
  fdup(p);

  goto next;

 op_ld_nul:
  if ( unlikely(rlp_chk_push(p, RUN_ERR)) )
    goto error;

  fpush(p, NUL);

  goto next;

 op_ld_true:
  if ( unlikely(rlp_chk_push(p, RUN_ERR)) )
    goto error;

  fpush(p, TRUE);

  goto next;

 op_ld_false:
  if ( unlikely(rlp_chk_push(p, RUN_ERR)) )
    goto error;

  fpush(p, FALSE);

  goto next;

 op_ld_zero:
  if ( unlikely(rlp_chk_push(p, RUN_ERR)) )
    goto error;

  fpush(p, ZERO);

  goto next;

 op_ld_one:
  if ( unlikely(rlp_chk_push(p, RUN_ERR)) )
    goto error;

  fpush(p, ONE);

  goto next;

 op_ld_fun:
  if ( unlikely(rlp_chk_push(p, RUN_ERR)) )
    goto error;

  fpush(p, tag(p->fn));

  goto next;

 op_ld_env:
  if ( unlikely(rlp_chk_push(p, RUN_ERR)) )
    goto error;

  fpush(p, tag(rlp_env(p)));

  goto next;

 op_ld_s16:
  if ( unlikely(rlp_chk_push(p, RUN_ERR)) )
    goto error;
  
  argx = *p->ip++;

  fpush(p, tag(argx));

  goto next;

 op_ld_s32:
  if ( unlikely(rlp_chk_push(p, RUN_ERR)) )
    goto error;

  argx = fetch32(&p->ip);

  fpush(p, tag(argx));

  goto next;

 op_ld_g16:
  if ( unlikely(rlp_chk_push(p, RUN_ERR)) )
    goto error;

  argx = *p->ip++;

  fpush(p, tag_glyph(argx));

  goto next;

 op_ld_g32:
  if ( unlikely(rlp_chk_push(p, RUN_ERR)) )
    goto error;

  argx = fetch32(&p->ip);

  fpush(p, tag_glyph(argx));

  goto next;

 op_ld_val:
  if ( unlikely(rlp_chk_push(p, RUN_ERR)) )
    goto error;

  argx = *p->ip++;
  tmpx = p->cl->vals->tail[argx];

  rlp_push(p, tmpx);

  goto next;

 op_ld_stk:
  if ( unlikely(rlp_chk_push(p, RUN_ERR)) )
    goto error;

  argx = *p->ip++;
  tmpx = rlp_peek(p, p->bp+argx);

  rlp_push(p, tmpx);

  goto next;

 op_put_stk:
  argx = *p->ip++;
  tmpx = rlp_pop(p);

  rlp_poke(p, p->bp+argx, tmpx);

  goto next;

 op_ld_upv:
  if ( unlikely(rlp_chk_push(p, RUN_ERR)) )
    goto error;
  
  argx = *p->ip++;
  upv  = cl_upvs(p)[argx];
  spc  = dr_upv(upv);
  tmpx = *spc;

  rlp_push(p, tmpx);

  goto next;

 op_put_upv:
  argx = *p->ip++;
  tmpx = fpop(p);
  upv  = cl_upvs(p)[argx];
  spc  = dr_upv(upv);
  *spc = tmpx;

  goto next;

 op_ld_cns:
  argy = -1;
  argx = *p->ip++;

  goto do_get_ns_ref;

 op_put_cns:
  argy = -1;
  argx = *p->ip++;

  goto do_put_ns_ref;

 op_ld_qns:
  argy = *p->ip++;
  argx = *p->ip++;

  goto do_get_ns_ref;

 op_put_qns:
  argy = *p->ip++;
  argx = *p->ip++;

  goto do_put_ns_ref;

 do_get_ns_ref:
  if ( unlikely(rlp_chk_push(p, RUN_ERR)) )
    goto error;

  ref = get_ns_ref(p, argy, argx);

  if ( rlp_chk_def(p, EVAL_ERR, ref) ) // check that reference is defined
    goto error;

  tmpx = ref->val;

  fpush(p, tmpx);

  goto next;

 do_put_ns_ref:
  ref = get_ns_ref(p, argy, argx);

  if ( rlp_chk_def(p, EVAL_ERR, ref) ) // check that reference is defined
    goto error;

  if ( rlp_chk_refv(p, EVAL_ERR, ref) ) // check that reference can be assigned
    goto error;

  if ( rlp_chk_reft(p, EVAL_ERR, ref, tmpx) )
    goto error;

  ref->val = tmpx;

  if ( ref->final ) { // infer type
    ref->tag = type_of(tmpx);
    set_meta(ref, ":tag", tag(ref->tag));
  }

  goto next;

 op_hndl:
  tmpx = fpop(p); // handler
  tmpy = fpop(p); // body

  if ( rlp_chk_type(p, EVAL_ERR, &ProtoType, tmpx, &h) )
    goto error;

  if ( rlp_chk_type(p, EVAL_ERR, &ProtoType, tmpy, &c) )
    goto error;

  if ( unlikely(rlp_chk_grows(p, RUN_ERR, 5)) )
    goto error;

  install_hndl(p, c, h);

  goto next;

 op_raise:
  
 op_catch:
  
 op_throw:
  
}

/* External APIs */
Error rl_exec(RlProc* p, Proto* c, Val* r) {
  reset_rlp(p);
  install_proto(p, c, -1);
  Error o;

  if ( (o=rl_exec_at(p, r, OP_ENTER)) )
    *r = NOTHING;

  reset_rlp(p);

  return o;
}
