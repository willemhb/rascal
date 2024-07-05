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

static void save_caller(Proc* p);

static void restore_caller(Proc* p, Val* bp, Val* cs) {
  if ( bp == NULL )
    bp = p->bp;
  
  if ( cs == NULL )
    cs = bp+p->fn->vc;
  
  Val    f = bp[-1];
  Val    t = tag_of(f);
  ushort o = untagv(f);

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

  Val* a = p->sp - c->ac;

  if ( t == -1 ) { // not a tail call
    pr_reserve(p, NOTHING, c->lc); // reserve space for other locals
    save_caller(p);                // save caller state

    // install registers
    a[-1]    = tagv(a-p->bp, PROTO_F);
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
    mov_tail_args(p, c->ac);                  // rebind arguments
    pr_initf(p, p->bp+c->ac, c->lc, NOTHING); // initialize other locals

    // install registers
    p->ip    = c->start;
    p->nx    = L_NEXT;
  }
}

static void install_catch(Proc* p, Proto* b, Proto* h) {
  ushort o;

  // prepare body for execution
  pr_push(p, NOTHING);          // dummy
  install_closure(p, b, -1);

  // compute offset to current handle pointer and save handle pointer
  o = p->hp == NULL ? 0 : p->sp-p->hp;
  pr_push(p, tag(o));

  // compute offset to current catch pointer and save catch pointer
  o = p->cp == NULL ? 0 : p->sp-p->cp;
  pr_push(p, tag(o));

  // save handler
  pr_push(p, tag(h));

  // save offset to caller state (becomes new catch pointer)
  p->cp = pr_push(p, tagv(p->sp-p->bp, CATCH_F));
}

static void install_hndl(Proc* p, Proto* b, Proto* h) {
  ushort o;

  // prepare body for execution
  pr_push(p, NOTHING);          // dummy
  install_closure(p, b, -1);

  // compute offset to current handle pointer and save handle pointer
  o = p->hp == NULL ? 0 : p->sp-p->hp;
  pr_push(p, tag(o));

  // compute offset to current catch pointer and save catch pointer
  o = p->cp == NULL ? 0 : p->sp-p->cp;
  pr_push(p, tag(o));

  // save handler
  pr_push(p, tag(h));

  // save offset to caller state (becomes new hndl pointer)
  p->hp = pr_push(p, tagv(p->sp-p->bp, HNDL_F));
}

static bool restore_catch(Proc* p) {
  bool r = p->cp == NULL;
  ushort o;

  if ( !r ) {
    Val*   cs = p->cp;
    Val*   bp;
    Val*   fs;
    Val*   rcp;
    Val*   rhp;
    Proto* h;
    Val    t;
    
    // compute the body's base pointer and the caller state to restore
    t  = tag_of(cs[0]);
    o  = untagv(cs[0]);
    bp = cs - o;
    fs = t == PROTO_F ? cs - 6 : cs - 7;

    // get handler
    h  = as_proto(cs[-1]);

    // compute catch and handle pointers to restore
    o   = untagv(cs[-2]);
    rcp = o == 0 ? NULL : cs - 2 - o;
    o   = untagv(cs[-3]);
    rhp = o == 0 ? NULL : cs - 3 - o;

    // close any upvalues that might be trashed by the catch
    close_upvs(&p->ou, bp);

    // restore caller (TODO: probably extremely optimazable, since the handler is immediately applied)
    restore_caller(p, bp, fs);

    // restore cp and hp
    p->cp = rcp;
    p->hp = rhp;

    // push handler in preparation for application
    p->sp[-1] = tag(h);
  }

  return r;
}

static void apply_throw(Proc* p, Sym* e, Str* m, Val b);

/* Internal */
Error rl_exec_at(Proc* p, Val* r, Label e) {

  // local macros
  // accessor macros  
#define fetch16(d)       ((d) = *(p)->ip++)
#define fetch32(d)       ((d) = do_fetch32(p))
#define mov(d, s)        ((d) = (s))
#define movd(d, s)       ((d) = untag((d), (s)))
#define mova(d, s)       (*(d) = (s))
#define movf(d, f, ...)  ((d) = f(__VA_ARGS__))

#define rx(rx)         (p->rx)
#define pushrx(rx)     pr_push(p, p->rx)
#define dpushrx(rx)    pr_push(p, tag(p->rx))
#define getrx(rx, d)   ((d) = p->rx)
#define vgetrx(rx, v)  ((v) = tag(p->rx))
#define movrx(rx, s)   (p->rx = (s))
#define vmovrx(rx, v)  (p->rx = untag(p->rx, v))
#define branch(t, n)   if ( (t) ) p->ip += (n)

#define stk(i)         (p->stk[i])
#define gets(i, v)    ((v) = stk(i))
#define dgets(i, d)   ((d) = untag((d), stk(i)))
#define movs(i, s)    (stk(i) = (s))
#define dmovs(i, d)   (stk(i) = tag(d))

#define loc(i)         (p->bp[i])
#define getl(i, v)     ((v) = loc(i))
#define dgetl(i, d)    ((d) = untag((d), loc(i)))
#define movl(i, v)     (loc(i) = (v))
#define dmovl(i, d)    (loc(i) = tag(d))

#define val(i)         (p->vs->tail[i])
#define getv(i, v)     ((v) = val(i))
#define dgetv(i, d)    ((d) = untag((d), val(i)))

#define upv(i)         (p->nv->upvals[i])
#define getu(i, v)     ((v) = *dr_upv(upv(i)))
#define dgetu(i, d)    ((d) = untag((d), *dr_upv(upv(i))))
#define movu(i, v)     (*dr_upv(upv(i)) = (v))
#define dmovu(i, d)    (*dr_upv(upv(i)) = tag(d))

#define dup()        pr_dup(p)
#define push(x)      pr_push(p, x)
#define dpush(x)     pr_push(p, tag(x))
#define pop()        pr_pop(p)
#define apop(a)      (*(a) = pr_pop(p))
#define dpopa(d)     (*(a) = untag(*(a), pr_pop(p)))
#define dpop(d)      ((d) = untag((d), pr_pop(p)))
#define vpop(v)      ((v) = pr_pop(p))

  // jump macros
#define jmp(l)     goto l
#define jmpn()     goto *next
#define jmpl(l)    goto *labels[l]
#define jmpe(p)    goto *labels[p->err]

  // errror macros
#define prne() pr_prn_err(p)

#define recover(o, m, v) pr_recover(p, o, m, v)

#define chkpop()                                   \
  if ( unlikely(pr_chk_pop(p, E_RUN)) ) jmpe(p) 

#define chkpush()                                             \
  if ( unlikely(pr_chk_push(p, E_RUN)) ) jmpe(p)

#define chkpushn(n)                                 \
  if ( unlikely(pr_chk_pushn(p, E_EVAL, n)) ) jmpe(p)

#define chkstkn(n)                                  \
  if ( unlikely(pr_chk_stkn(p, E_RUN, n)) ) jmpe(p)
  
#define chkdef(r)                                   \
  if ( unlikely(pr_chk_def(p, E_EVAL, r)) ) jmpe(p)

#define chkrefv(r)                                  \
  if ( unlikely(pr_chk_refv(p, E_EVAL, r)) ) jmpe(p)
  
#define chkreft(r, g)                                      \
  if ( unlikely(pr_chk_reft(p, E_EVAL, r, g)) ) jmpe(p)
  
#define chktype(x, g, s)                                   \
  if ( unlikely(pr_chk_type(p, E_EVAL, x, g, s)) ) jmpe(p)

#define pushnx(l)                                \
  do {                                            \
    if ( rx(nx) < L_READY ) {                    \
      chkpush();                                 \
      dpushrx(nx);                               \
    }                                             \
    movrx(nx, l);                               \
    mov(next, labels[l]);                       \
  } while (false)

#define popnx()                                \
  do {                                          \
    poprx(nx);                                  \
    mov(next, labels[reg(nx)]);                 \
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

  pushnx(e);
  jmpn();

  /* error labels */
  // not an error
 e_okay:
  jmpn();

  // unrecoverable errors
 e_sys:
 e_run:
 e_norecover:
  prne();
  mova( r, NOTHING );
  jmp( end );

 e_read:
 e_comp:
 e_stx:
 e_genfn:
 e_eval:
 e_user:
  if ( restore_catch(p) )
    jmp(e_norecover);

  recover(&ox, &mx, &vx);     // clear error
  apply_throw(p, ox, mx, vx); // apply arguments
  jmpn();                     // jump to handler

 l_next:
  fetch16(o);             // get next opcode
  jmpl(o);               // dispatch to label

  /* opcode labels */
 o_noop:                 // dummy operation (does nothing)
  jmpn();

 o_pop:                  // removes TOS
  chkpop();             // check underflow
  pop();                 // remove TOS
  jmpn();               // fetch next instruction

 o_dup:                  // duplicates TOS
  chkstkn(1);           // check underflow (have to have something to duplicate)
  chkpush();            // check overflow
  dup();                 // duplicate TOS
  jmpn();               // fetch next instruction

 o_ld_nul:               // adds NUL to stack
  chkpush();            // check overflow
  push(NUL);             // push NUL
  jmpn();               // fetch next instruction

 o_ld_true:              // adds TRUE to stack
  chkpush();            // check overflow
  push(TRUE);            // push TRUE
  jmpn();               // fetch next instruction

 o_ld_false:             // adds FALSE to stack
  chkpush();            // check overflow
  push(FALSE);           // push FALSE
  jmpn();               // fetch next instruction

 o_ld_zero:              // adds ZERO to stack
  chkpush();            // check overflow
  push(ZERO);            // push ZERO
  jmpn();               // fetch next instruction

 o_ld_one:               // adds ONE to stack
  chkpush();            // check overflow
  push(ONE);             // push ONE
  jmpn();               // fetch next instruction

 o_ld_fun:               // loads currently executing function
  chkpush();            // check overflow
  dpushrx(fn);          // push p->fn
  jmpn();               // fetch next instruction

 o_ld_env:               // loads current environment
  chkpush();            // check overflow
  dpushrx(nv);          // push p->nv
  jmpn();               // fetch next instruction

 o_ld_s16:               // loads 16-bit Small inlined in bytecode
  chkpush();            // check overflow
  fetch16(ax);           // fetch inlined small
  dpush(ax);            // add to stack
  jmpn();               // fetch next instruction

 o_ld_s32:               // loads 32-bit Small inlined in bytecode
  chkpush();            // check overflow
  fetch32(ax);           // fetch inlined Small from bytecode
  dpush(ax);            // add to stack
  jmpn();               // fetch next instruction

 o_ld_g16:               // loads 16-bit Glyph inlined in bytecode
  chkpush();            // check overflow
  fetch16(gx);           // fetch inlined Glyph from bytecode
  dpush(gx);            // add to stack
  jmpn();               // fetch next instruction

 o_ld_g32:               // loads 32-bit Glyph inlined in bytecode
  chkpush();            // check overflow
  fetch32(gx);           // fetch inlined Glyph from bytecode
  dpush(gx);            // add to stack
  jmpn();               // fetch next instruction

 o_ld_val:               // loads value from Proto constant store
  chkpush();            // check overflow
  fetch16(ax);           // fetch offset
  getv(ax, vx);       // fetch value
  push(vx);              // add to stack
  jmpn();               // fetch next instruction

 o_ld_stk:
  chkpush();
  fetch16(ax);
  getl(ax, vx);
  push(vx);
  jmpn();

 o_put_stk:               // 
  fetch16(ax);
  vpop(vx);
  jmpn();

 o_ld_upv:
  chkpush();
  fetch16(ax);
  getu(ax, vx);
  push(vx);
  jmpn();

 o_put_upv:
  fetch16(ax);
  vpop(vx);
  movu(ax, vx);
  jmpn();

 o_ld_cns:
  mov(ay, -1);
  fetch16(ax);
  jmp(do_get_ns_ref);

 o_put_cns:
  mov(ay, -1);
  fetch16(ax);
  jmp(do_get_ns_ref);

 o_ld_qns:
  fetch16(ay);
  fetch16(ax);
  jmp(do_get_ns_ref);

 o_put_qns:
  fetch16(ay);
  fetch16(ax);
  jmp(do_put_ns_ref);

 do_get_ns_ref:
  chkpush();
  movf(rx, get_ns_ref, p, ay, ax);
  chkdef(rx);
  mov(vx, rx->val);
  push(vx);
  jmpn();

 do_put_ns_ref:
  movf( rx, get_ns_ref, p, ay, ax );
  chkdef( rx );
  chkrefv( rx );
  chkreft( rx, vx );
  mov( rx->val, vx );

  if ( rx->final ) { // infer type
    movf( rx->tag, type_of, vx );
    set_meta( rx, ":tag", tag(rx->tag) );
  }

  jmpn();

 o_hndl:
  vpop(vx);                      // fetch handler
  vpop(vy);                      // fetch body
  chktype(&ProtoType, vx, &hx);  // validate type of handler
  chktype(&ProtoType, vy, &bx);  // validate type of body
  chkpushn(bx->vc + 2);          // check stack overflow
  install_hndl(p, bx, hx);       // install body and handler
  jmpn();                        // jump to body

 o_raise1:
  mov( vx, NOTHING );    // operation continuation (ignored)
  mov( vz, NOTHING );    // operation argument (ignored)
  vpop( vy );            // operation label (must be Sym)
  jmp( do_raise );

 o_raise2:
  mov( vx, NOTHING );
  vpop( vy );
  vpop( vz );

  if ( is_cntl(vy) ) {
    mov( vx, vy );
    mov( vy, vz );
    mov( vz, NOTHING );
  }

  jmp( do_raise );
  
 o_raise3:
  vpop( vx );     // operation continuation (must be Cntl)
  vpop( vy );     // operation label (must be Sym)
  vpop( vz );     // operation argument (can be anything)

 do_raise:
  if ( vx == NOTHING )
    mov( kx, NULL );

  else
    chktype( &CntlType, vx, &kx );

  chktype( &SymType, vy, &ox );
  
 o_catch:
  
 o_throw1:
 o_throw2:
 o_throw3:
  
 o_jmp:
 fetch16( ax );
 branch( true, ax );             // constant folding probably removes this lol
 jmpn();

 o_jmpt:
  fetch16( ax );
  vpop( vx );
  branch( truthy(vx), ax );
  jmpn();

 o_jmpf:
  fetch16( ax );
  vpop( vx );
  branch( falsey(vx), ax );
  jmpn();

 o_closure:
  fetch16( ax );              // number of uxalues to be initialized
  dgets( -1, cx );            // fetch prototype of new closure
  movf( cx, bind_proto, cx ); // create bound prototype (aka closure)
  dmovs( -1, cx );            // return to stack in case allocating uxalues triggers gc

  // initialize uxalues
  for ( int i=0; i < ax; i++ ) {
    fetch16( ay ); // local?
    fetch16( az ); // offset

    if ( ay )
      cl_upvs(cx)[i] = open_upv(&rx(ou), &loc(az));

    else
      cl_upvs(cx)[i] = upv(az);
  }

  jmpn();

 o_capture:                      // captures 
  close_upvs(&rx(ou), rx(bp));
  jmpn();

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
  gets( -1, vx );
  restore_caller( p, NULL, NULL ); // restore caller
  push( vx );                      // push return value
  jmpn();                          // jump to caller

 o_exit:
  goto end;

 f_typeof:
  /* Get the data type of TOS */
  gets( -1, vx );
  movf( tx, type_of, vx );
  dmovs( -1, tx );
  jmpn();

 f_hash:
  gets( -1, vx );
  movf( vx, rl_hash, vx, false );
  dmovs( -1, vx );
  jmpn();

 f_same:
  vpop( vx );
  gets( -1, vy );
  movs( -1, vx == vy ? TRUE : FALSE );
  jmpn();

 end:
  if ( rx(err) == E_OKAY )
    apop(r);

  return p->err;

#undef chk_pop
#undef chk_push
#undef chk_pushn
#undef chk_stkn
#undef chk_def
#undef chk_refv
#undef chk_reft
#undef chk_type
#undef fetch16
#undef fetch32
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
