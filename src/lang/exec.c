#include <string.h>

#include "lang/exec.h"

#include "val/function.h"
#include "val/environ.h"

#include "vm/interpreter.h"
#include "vm/environ.h"

#include "opcodes.h"

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

/* External APIs */
// pseudo-regisers
#define FUN    Vm.ex
#define ERR    Vm.err
#define IP     Vm.ip
#define FSIZE  Vm.fs
#define OPENS  Vm.upv
#define ENV    FUN->envt
#define VALS   FUN->vals->tail
#define UPVALS ENV->upvals
#define ARGS   Vm.bp
#define TOS    Vm.stk.data[Vm.stk.cnt-1]
#define CNS    ENV->ns->refs->data

#define RESET()             reset_is(&Vm)
#define FINISHED()          (Vm.ex == NULL)
#define INIT(c)             init_is(&Vm, c)
#define CHKARGC(f, x, g, v) is_chkargc(&Vm, EVAL_ERROR, f, x, g, v)
#define CHKTYPE(f, x, g)    is_chktype(&Vm, EVAL_ERROR, f, x, type_of(g))
#define CAPTURE(l)          is_capture(&Vm, l)
#define CLOSE(l)            is_close(&Vm, l)
#define POP(c)              is_pop(&Vm, c)
#define POPN(n, c)          is_popn(&Vm, n, c)
#define PUSH(x, c)          is_push(&Vm, x, c)
#define DUP(c)              is_dup(&Vm, c)
#define PUSHF(x, c)         is_pushf(&Vm, x, c)
#define POPF(c)             is_popf(&Vm, c)
#define PUSHC(f, h, c)      is_pushc(&Vm, f, h, c)
#define PUSHH(f, h, c)      is_pushh(&Vm, f, h, c)

#define FETCH()     (*IP++)
#define FETCH32()   fetch32(&IP)
#define DECODE()    goto *labels[op]
#define JUMP(x)     (IP += (x))
#define CONTINUE()  goto next
#define ABORT()     goto end
#define EXIT()      goto end
#define MOVE(d, s)  ((d) = (s))
#define BEGIN()     goto top

rl_err_t rl_exec(Closure* c, Val* b) {
  static void* labels[] = {
    [OP_NOOP]     = &&op_noop,

    [OP_POP]      = &&op_pop,
    [OP_DUP]      = &&op_dup,
    [OP_POPF]     = &&op_popf,

    [OP_LD_NUL]   = &&op_ld_nul,
    [OP_LD_TRUE]  = &&op_ld_true,
    [OP_LD_FALSE] = &&op_ld_false,
    [OP_LD_ZERO]  = &&op_ld_zero,
    [OP_LD_ONE]   = &&op_ld_one,

    [OP_LD_FUN]   = &&op_ld_fun,
    [OP_LD_ENV]   = &&op_ld_env,

    [OP_LD_S16]   = &&op_ld_s16,
    [OP_LD_S32]   = &&op_ld_s32,
    [OP_LD_G16]   = &&op_ld_g16,
    [OP_LD_G32]   = &&op_ld_g32,

    [OP_LD_VAL]   = &&op_ld_val,

    [OP_LD_STK]   = &&op_ld_stk,
    [OP_PUT_STK]  = &&op_put_stk,

    [OP_LD_UPV]   = &&op_ld_upv,
    [OP_PUT_UPV]  = &&op_put_upv,

    [OP_LD_CNS]   = &&op_ld_cns,
    [OP_PUT_CNS]  = &&op_ld_cns,

    [OP_JMP]      = &&op_jmp,
    [OP_JMPT]     = &&op_jmpt,
    [OP_JMPF]     = &&op_jmpf,

    [OP_CLOSURE]  = &&op_closure,
    [OP_CAPTURE]  = &&op_capture,
    
    [OP_RETURN]   = &&op_return,

    [FN_EXEC]     = &&fn_exec,
  };

  rl_err_t o;
  OpCode op;
  int argx, argy, argz;
  Val tmpx, tmpy;
  UpVal* upv;
  Val* spc;
  Ref* ref;
  Closure* h;

 top:
  is_pushf(&Vm, c, true);

 next:
  if ( Vm.err )
    goto end;

  op = *Vm.ip++;
  goto *labels[op];

 end:
  o = Vm.err;

  if ( o == OKAY )
    *b = is_pop(&Vm, true);

  reset_is(&Vm);

  return o;

 op_noop:
  goto next;

 op_pop:
  is_pop(&Vm, true);
  goto next;

 op_dup:
  is_dup(&Vm, true);
  goto next;

 op_popf:
  tmpx = is_tos(&Vm, false);
  is_popn(&Vm, Vm.fs, false);
  is_push(&Vm, tmpx, false);
  goto next;

 op_ld_nul:
  PUSH(NUL, true);
  CONTINUE();

 op_ld_true:
  PUSH(TRUE, true);
  CONTINUE();

 op_ld_false:
  PUSH(FALSE, true);
  CONTINUE();

 op_ld_zero:
  PUSH(ZERO, true);
  CONTINUE();

 op_ld_one:
  PUSH(ONE, true);
  CONTINUE();

 op_ld_fun:
  MOVE(tmpx, tag(FUN));
  PUSH(tmpx, true);
  CONTINUE();

 op_ld_env:
  MOVE(tmpx, tag(ENV));
  PUSH(tmpx, true);
  CONTINUE();

 op_ld_s16:
  MOVE(argx, FETCH());
  MOVE(tmpx, tag_small(argx));
  PUSH(tmpx, true);
  CONTINUE();

 op_ld_s32:
  MOVE(argx, FETCH32());
  MOVE(tmpx, tag_small(argx));
  PUSH(tmpx, true);
  CONTINUE();

 op_ld_g16:
  MOVE(argx, FETCH());
  MOVE(tmpx, tag_glyph(argx));
  PUSH(tmpx, true);
  CONTINUE();

 op_ld_g32:
  MOVE(argx, FETCH32());
  MOVE(tmpx, tag_glyph(argx));
  PUSH(tmpx, true);
  CONTINUE();
  
 op_ld_val:
  MOVE(argx, FETCH());
  MOVE(tmpx, VALS[argx]);
  PUSH(tmpx, true);
  CONTINUE();

 op_ld_stk:
  MOVE(argx, FETCH());
  MOVE(tmpx, ARGS[argx]);
  PUSH(tmpx, true);
  CONTINUE();

 op_put_stk:
  MOVE(argx, FETCH());
  MOVE(ARGS[argx], TOS);
  CONTINUE();

 op_ld_upv:
  MOVE(argx, FETCH());
  MOVE(upv, UPVALS[argx]);
  MOVE(spc, dr_upv(upv));
  MOVE(tmpx, *spc);
  PUSH(tmpx, true);
  CONTINUE();

 op_put_upv:
  MOVE(argx, FETCH());
  MOVE(upv, UPVALS[argx]);
  MOVE(spc, dr_upv(upv));
  MOVE(*spc, TOS);
  CONTINUE();

 op_ld_cns:
  MOVE(argx, FETCH());
  MOVE(ref,  CNS[argx]);
  MOVE(tmpx, ref->val);
  PUSH(tmpx, true);
  CONTINUE();

 op_jmp:
  MOVE(argx, FETCH());
  JUMP(argx);
  CONTINUE();

 op_jmpt:
  MOVE(argx, FETCH());
  MOVE(tmpx, POP(true));

  if ( truthy(tmpx) )
    JUMP(argx);

  CONTINUE();

 op_jmpf:
  MOVE(argx, FETCH());
  MOVE(tmpx, POP(true));

  if ( falsey(tmpx) )
    JUMP(argx);

  CONTINUE();

 op_closure:
  // create a copy of the template 
  MOVE(argx, FETCH());
  MOVE(tmpx, POP(true));
  MOVE(c, as_cls(tmpx));
  MOVE(c, clone_obj(c));

  for ( int i=0; i<argx; i++ ) {
    argy = FETCH(); // local?
    argz = FETCH(); // index

    if ( argy )
      upv = CAPTURE(&ARGS[argz]);

    else
      upv = UPVALS[argz];

    set_upv(c, upv, i);
  }

  MOVE(tmpx, tag(c));
  PUSH(tmpx, true);
  CONTINUE();

 op_capture:
  close_upvs(&OPENS, ARGS);
  CONTINUE();

 op_catch:
  MOVE(tmpy, POP(false)); // catch body
  MOVE(tmpx, POP(false)); // catch handler
  MOVE(c, as_cls(tmpx));
  MOVE(h, as_cls(tmpy));
  PUSHC(c, h, true);
  CONTINUE();

 op_handle:
  MOVE(tmpy, POP(false)); // hndl body
  MOVE(tmpx, POP(false)); // hndl handler
  MOVE(c, as_cls(tmpx));
  MOVE(h, as_cls(tmpy));
  PUSHH(c, h, true);

 op_return:
  /* Appropriate stack manipulations handled by OP_POPF instruction, this just restores the caller. */
  POPF(true);

  if ( FINISHED() )
    EXIT();

  FSIZE++; // account for return value
  MOVE(ARGS, );
  CONTINUE();

  // primitive functions (separate from native functions mostly to re-use C stack)
 fn_exec:
  MOVE(tmpx, POP(true));
  CHKTYPE("exec", &ClosureType, tmpx);
  MOVE(c, as_cls(tmpx));
  BEGIN();

  
}
