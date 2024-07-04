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

static Ref* get_ns_ref(Proc* p, int yoff, int xoff) {
  Env* nv;
  Ref* o = NULL;

  if ( yoff == -1 ) {
    nv = p->nv->ns;

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

static inline int do_fetch32(Proc* p) {
  int b;

  memcpy(&b, p->ip, sizeof(int));
  p->ip += 2;

  return b;
}

static void save_caller(Proc* p, size_t n);

static void restore_caller(Proc* p) {
  Val*  bp = p->bp;
  Val    f = bp[-1];
  Val    t = tag_of(f);
  ushort o = untagv(f);
  Val*  cs = p->bp+p->fn->vc;

  if ( t == PRIM_F ) {
    p->fn    = as_func(cs[0]);
    p->nv    = as_env(cs[1]);
    p->nx    = as_small(cs[2]);

  } else {
    Proto* c = as_proto(cs[0]);
    p->fn    = (Func*)c;
    p->nv    = c->envt;
    p->vs    = c->vals;
    p->ip    = as_ptr(cs[1]);
    p->nx    = L_NEXT;
  }

  p->bp -= o;
  p->sp  = bp;                    // NB: this leaves space for return value
}

static void resize_args(Proc* p, size_t c, size_t n) {
  if ( c < n )
    pr_pushat(p, p->bp+c, n-c, NOTHING);

  else if ( c > n )
    pr_popat(p, p->bp+n, c-n);
}

static void resize_locals(Proc* p, size_t a, size_t c, size_t n) {
  if ( c < n )
    pr_pushat(p, p->bp+a+c, n-c, NOTHING);

  else if ( c > n )
    pr_popat(p, p->bp+a+n, c-n);
}

static void resize_vars(Proc* p, void* o) {
  Func* f = o;
  size_t oac = p->fn->ac;
  size_t olc = p->fn->lc;
  size_t nac = f->ac;
  size_t nlc = f->lc;

  resize_args(p, oac, nac);
  resize_locals(p, nac, olc, nlc);
}

static void mov_tail_args(Proc* p, size_t n) {
  pr_move(p, p->bp, p->sp-n, n);
  p->sp -= n+1;
}

static void install_closure(Proc* p, Proto* c, int t) {
  /* NB: function arguments have already been validated.

     Stack overflow has already been checked.
     
     `t` indicates frame reuse for tail calls. */

  Val* a = p->stk - c->ac;

  if ( t == -1 ) { // not a tail call
    pr_reserve(p, NOTHING, c->lc); // reserve space for other locals
    save_caller(p, c->vc);         // save caller state

    // install registers
    p->fn    = (Func*)c;
    p->ip    = c->start;
    p->bp    = a;
    p->nv    = c->envt;
    p->vs    = c->vals;
    p->nx    = L_NEXT;

  } else if ( t == 0 ) { // general tail call - frame may need to be resized
    resize_vars(p, c);                         // resize frame
    mov_tail_args(p, c->ac);                   // copy arguments (also pops arguments)
    pr_initf(p, p->bp+c->ac, c->lc, NOTHING);  // initialize other locals

    // install registers
    p->fn    = (Func*)c;
    p->ip    = c->start;
    p->nv    = c->envt;
    p->vs    = c->vals;
    p->nx    = L_NEXT;

  } else { // tail recursive call - frame can be assumed to be correct size
    mov_tail_args(p, c->ac);                   // rebind arguments
    pr_initf(p, p->bp+c->ac, c->lc, NOTHING);  // initialize other locals

    // install registers
    p->ip    = c->start;
    p->nx    = L_NEXT;
  }
}

static void install_catch(Proc* p, Proto* b, Proto* h) {
  ushort o;

  install_closure(p, b, -1);               // prepare body for execution
  pr_push(p, tag(h));                      // save handler

  o     = p->rp == NULL ? 0 : p->sp-p->rp; // compute offset to current rp (0 means it doesn't exist)
  p->rp = pr_push(p, tagv(o, CATCH_F));
}

static void install_hndl(Proc* p, Proto* b, Proto* h) {
  ushort o;

  install_closure(p, b, -1);               // prepare body for execution
  pr_push(p, tag(h));                      // save handler

  o     = p->rp == NULL ? 0 : p->sp-p->rp; // compute offset to current rp (0 means it doesn't exist)
  p->rp = pr_push(p, tagv(o, HNDL_F));     // set new rp
}

static Val* find_catch_rp(Proc* p) {
  /* Traverse stack to find nearest enclosing frame installed by `catch`.

     This is necessary because both `catch` and */
  Val* rp = p->rp;

  while ( rp && tag_of(*rp) != CATCH_F ) {
    ushort o = *rp & WDATA_BITS;

    if ( o == 0 )
      rp = NULL;

    else
      rp = rp - o;
  }

  return rp;
}

static bool restore_catch(Proc* p, Proto** h) {
  Val* f = find_catch_rp(p);
  bool o = f == NULL;
  
  if ( !o ) {
    
  }

  return o;
}

/* Internal */
Error rl_exec_at(Proc* p, Val* r, Label e) {

  // local macros
  // accessor macros  
#define fetch16(d)    ((d) = *(p)->ip++)
#define fetch32(d)    ((d) = do_fetch32(p)) 
#define reg(rx)       (p->rx)
#define stk(i)        (p->stk[i])
#define loc(i)        (p->bp[i])
#define vals(i)       (p->vs->tail[i])
#define upvs(i)       (p->nv->upvs[i])
#define mov(d, s)     ((d) = (s))
#define mov_rx(d, rx) ((d) = p->rx)

  // jump macros
#define jmp(l)     goto l
#define jmp_n()    goto *next
#define jmp_l(l)   goto *labels[l]
#define jmp_e(p)   goto *labels[p->err]

  // errror macros
#define chk_pop()                                   \
  if ( unlikely(pr_chk_pop(p, E_RUN)) ) jmp_e(p) 

#define chk_push()                                             \
  if ( unlikely(pr_chk_push(p, E_RUN)) ) jmp_e(p)

#define chk_pushn(n)                                 \
  if ( unlikely(pr_chk_pushn(p, E_EVAL, n)) ) jmp_e(p)

#define chk_stkn(n)                                  \
  if ( unlikely(pr_chk_stkn(p, E_RUN, n)) ) jmp_e(p)
  
#define chk_def(r)                                   \
  if ( unlikely(pr_chk_def(p, E_EVAL, r)) ) jmp_e(p)

#define chk_refv(r)                                  \
  if ( unlikely(pr_chk_refv(p, E_EVAL, r)) ) jmp_e(p)
  
#define chk_reft(r, g)                                      \
  if ( unlikely(pr_chk_reft(p, E_EVAL, r, g)) ) jmp_e(p)
  
#define chk_type(x, g, s)                                   \
  if ( unlikely(pr_chk_type(p, E_EVAL, x, g, s)) ) jmp_e(p)

#define dup()       pr_dup(p)
#define push(x)     pr_push(p, x)
#define push_d(x)   pr_push(p, tag(x))
#define push_rx(rx) pr_push(p, tag(p->rx))
#define pop()       pr_pop(p)
#define pop_d(d)    ((d) = untag((d), pr_pop(p)))
#define pop_rx(rx)  (p->rx = untag((p->rx), p))

#define push_nx(l)                                \
  do {                                            \
    if ( reg(nx) < L_READY ) {                    \
      chk_push();                                 \
      push_rx(nx);                                \
    }                                             \
    mov(reg(nx), l);                              \
    mov(next, labels[l]);                         \
  } while (false)

#define pop_nx()                                \
  do {                                          \
    mov(reg(nx), as_small(pr_pop(p)));          \
    mov(next, labl(reg(nx)));                   \
  } while (false)


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
  Glyph gx;
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

  push_nx(e);
  jmp_n();

  /* error labels */
  // not an error
 e_okay:
  jmp_n();

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
  

  pr_recover(p, &ox, &mx, &vx);   // clear error
  apply_throw(p, hx, ox, mx, vx); // 
  jmp_n();

 l_next:
  fetch16(o);             // get next opcode
  jmp_l(o);               // dispatch to label

  /* opcode labels */
 o_noop:                 // dummy operation (does nothing)
  jmp_n();

 o_pop:                  // removes TOS
  chk_pop();             // check underflow
  pop();                 // remove TOS
  jmp_n();               // fetch next instruction

 o_dup:                  // duplicates TOS
  chk_stkn(1);           // check underflow (have to have something to duplicate)
  chk_push();            // check overflow
  dup();                 // duplicate TOS
  jmp_n();               // fetch next instruction

 o_ld_nul:               // adds NUL to stack
  chk_push();            // check overflow
  push(NUL);             // push NUL
  jmp_n();               // fetch next instruction

 o_ld_true:              // adds TRUE to stack
  chk_push();            // check overflow
  push(TRUE);            // push TRUE
  jmp_n();               // fetch next instruction

 o_ld_false:             // adds FALSE to stack
  chk_push();            // check overflow
  push(FALSE);           // push FALSE
  jmp_n();               // fetch next instruction

 o_ld_zero:              // adds ZERO to stack
  chk_push();            // check overflow
  push(ZERO);            // push ZERO
  jmp_n();               // fetch next instruction

 o_ld_one:               // adds ONE to stack
  chk_push();            // check overflow
  push(ONE);             // push ONE
  jmp_n();               // fetch next instruction

 o_ld_fun:               // loads currently executing function
  chk_push();            // check overflow
  push_rx(fn);           // push p->fn
  jmp_n();               // fetch next instruction

 o_ld_env:               // loads current environment
  chk_push();            // check overflow
  push_rx(nv);           // push p->nv
  jmp_n();               // fetch next instruction

 o_ld_s16:               // loads 16-bit Small inlined in bytecode
  chk_push();            // check overflow
  fetch16(ax);           // fetch inlined small
  push_d(ax);            // add to stack
  jmp_n();               // fetch next instruction

 o_ld_s32:               // loads 32-bit Small inlined in bytecode
  chk_push();            // check overflow
  fetch32(ax);           // fetch inlined small from bytecode
  push_d(ax); 
  jmp_n();

 o_ld_g16:
  chk_push(p);
  fetch16(ax, p);
  pr_fpush(p, tag_glyph(ax));
  jmp_n();

 o_ld_g32:
  chk_push(p);
  fetch32(ax, p);
  pr_fpush(p, tag_glyph(ax));
  jmp_n();

 o_ld_val:
  chk_push(p);
  fetch16(ax, p);
  val(vx, ax);
  pr_fpush(p, vx);
  jmp_n();

 o_ld_stk:
  chk_push(p);
  fetch16(ax, p);
  local(vx, ax);
  pr_fpush(p, vx);
  jmp_n();

 o_put_stk:
  fetch16(ax, p);
  mov(vx, pr_fpop(p));
  mov(local(ax), vx);

  jmp_n();

 o_ld_upv:
  chk_push(p);
  fetch16(ax, p);
  upval(ux, ax);
  mov(vxs, dr_upv(ux));
  mov(vx, *vxs);
  pr_fpush(p, vx);
  jmp_n();

 o_put_upv:
  ax   = fetch16(p);
  vx   = pr_fpop(p);
  ux   = pr_upvs(p)[ax];
  vxs  = dr_upv(ux);
  *vxs = vx;

  jmp_n();

 o_ld_cns:
  ay = -1;
  fetch16(ax, p);
  jmp(do_get_ns_ref);

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

  jmp_n();

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

  jmp_n();

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

  jmp_n();

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

  jmp_n();

 o_jmpt:
  ax   = fetch16(p);
  vx   = pr_fpop(p);

  if ( truthy(vx) )
    p->fp += ax;

  jmp_n();

 o_jmpf:
  ax = fetch16(p);
  vx = pr_fpop(p);

  if ( falsey(vx) )
    p->fp += ax;

  jmp_n();

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

  jmp_n();

 o_capture:                      // captures 
  close_upvs(&reg(ou), reg(bp));
  jmp_n();

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
  mov(vx, stk(-1)); // return value
  pr_restoref(p);    // restore caller
  pr_fpush(p, vx);   // push return value
  jmp_n();   // jump to caller

 o_exit:
  goto end;

 f_typeof:
  /* Get the data type of TOS */
  vx        = p->sp[-1];
  tx        = type_of(vx);
  p->sp[-1] = tag(tx);

  jmp_n();

 f_hash:
  vx        = p->sp[-1];
  vy        = rl_hash(vx, false);
  p->sp[-1] = tag_arity(vy);

  jmp_n();

 f_same:
  vx        = pr_fpop(p);
  vy        = p->sp[-1];
  vz        = vx == vy ? TRUE : FALSE;
  p->sp[-1] = vz;

  jmp_n();

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
