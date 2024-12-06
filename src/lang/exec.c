#include <string.h>

#include "lang/exec.h"
#include "lang/compare.h"

#include "val/alist.h"
#include "val/function.h"
#include "val/environ.h"
#include "val/text.h"

#include "vm/error.h"
#include "vm/state.h"

#include "util/number.h"

/* Internal APIs */
static inline bool truthy(Val x) {
  return x != FALSE && x != NUL;
}

static inline bool falsey(Val x) {
  return x == FALSE || x == NUL;
}

static inline Val* get_upval(Proc* p, size32 i) {
  return dr_upv(as_upv(p->code->cls->data[i]));
}

static inline Val* get_sref(Proc* p, int i) {
  return i < 0 ? p->sp + i : p->stk + i;
}

/* External APIs */
Error rl_exec(Proc* p, UserFn* c, Val* b) {

#define cname(p)           ((p)->code->name->n->cs)
#define ip(p)              ((p)->ip)
#define sp(p)              ((p)->sp)
#define stack(p, i)        (get_sref(p, i)[0])
#define constant(p, i)     ((p)->code->vals->data[i])
#define local(p, i)        (p->bp[i])
#define upval(p, i)        (get_upval(p, i)[0])
#define fetch(p)           (*((p)->ip++))
#define next(p)            goto *labels[(op = fetch(p))]

  void* labels[N_OPCODES] = {
    // stack manipulation
    [O_NOOP]   = &&op_noop,
    [O_POP]    = &&op_pop,

    // load/store instructions
    [O_LDT]    = &&op_ldt,
    [O_LDF]    = &&op_ldf,
    [O_LDN]    = &&op_ldn,
    [O_LDVAL]  = &&op_ldval,
    [O_LDSTK]  = &&op_ldstk,
    [O_PUTSTK] = &&op_putstk,

    // jump instructions
    [O_JMP]    = &&op_jmp,
    [O_JMPT]   = &&op_jmpt,
    [O_JMPF]   = &&op_jmpf,

    // error handling instructions
    [O_CATCH]  = &&op_catch,
    [O_ECATCH] = &&op_ecatch,
    [O_THROW]  = &&op_throw,

    // function call instructions
    [O_CALL]   = &&op_call,
  };

  Error status;
  Fn* fn;
  PrimFn* pf;
  UserFn* uf;
  Val x, y, z, v;
  Type tx;
  Opcode op;
  int rx, ry, rz, ac;

  // stack manipulation
 op_noop:
  next(p);

 op_pop:
  pop(p);
  next(p);

  // loads and stores
 op_ldt: // load constant value true
  push(p, TRUE);
  next(p);

 op_ldf: // load constant value false
  push(p, FALSE);
  next(p);

 op_ldn: // load constant value nul
  push(p, NUL);
  next(p);

 op_ldval: // load from closure's value store
  rx = fetch(p);
  x  = constant(p, rx);
  push(p, x);
  next(p);

 op_ldstk: // load a local variable stored on the stack
  rx = fetch(p);
  x  = local(p, rx);
  push(p, x);
  next(p);

 op_putstk: // store TOS to a local variable stored on the stack
  rx           = fetch(p);
  x            = pop(p);
  local(p, rx) = x;
  next(p);

  // jumps
 op_jmp: // unconditional jump
  rx     = fetch(p);
  ip(p) += rx;
  next(p);

 op_jmpt: // pop TOS and jump if the value is any value besides false or nul
  rx = fetch(p);
  x  = pop(p);

  if ( truthy(x) )
    ip(p) += rx;

  next(p);

 op_jmpf: // pop TOS and jump if the value is false or nul
  rx = fetch(p);
  x  = pop(p);

  if ( falsey(x) )
    ip(p) += rx;

  next(p);

  // exception handling instructions
 op_catch:             // 
  rl_epush(p);         // save current execution state
  status = rl_eset(p); // set jump buffer

  if ( status ) {
    ac = 1;            // set ac
    x  = pop(p);       // save TOS (argument to throw closure)
    rl_epop(p, true);  // restore execution state (throw closure is tos)
    push(p, x);        // push argument to throw onto the stack

    goto do_call;
  } else {
    
  }

 op_ecatch:
  rl_epop(p, false);
  next(p);

 op_throw:
  

 op_call:
  ac = fetch(p);

 do_call:
  x  = stack(p, -ac-1);                                    // caller
  fn = as_fn_s(p, cname(p), x);                            // validate type
  ac = rl_argco(p, fn_name(fn), ac, fn_va(fn), fn_ac(fn)); // validate arity

  if ( is_userfn(fn) )
    goto user_call;

  else
    goto prim_call;

 prim_call:
  pf            = as_primfn(fn);
  x             = pf->fn(p, ac); // apply C function
  sp(p)        -= ac;            // remove arguments from stack
  stack(p, -1)  = x;             // replace TOS (calling function) with result
  next(p);

  user_call:
  uf            = as_userfn(fn); // 
  
  next(p);

 end:
  if ( status == E_OKAY && b )
    *b = pop(p);

  return status;

#undef move
#undef fetch
#undef next
}
