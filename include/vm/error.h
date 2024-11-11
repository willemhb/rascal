#ifndef rl_vm_error_h
#define rl_vm_error_h

#include "labels.h"

#include "vm/state.h"

/* Helpers and typedefs for handling rascal errors. */
/* C types */
struct EFrame {
  /* Saved heap state */
  HFrame* hfs;

  /* Saved execution state */
  UserFn* code;
  sint16* ip;
  Val*    bp;
  Val*    sp;
  void**  fp;

  /* Saved C state */
  jmp_buf Cstate;
};

/* APIs */
/* External APIs */
// setjmp/longjmp helpers
#define rl_eset(p)    setjmp((p)->cp->Cstate)
#define rl_ejmp(p, c) longjmp((p)->cp->Cstate, (c))

// managing error state
void  rl_epush(Proc* p);        // save current Vm state
void  rl_epop(Proc* p, bool r); // restore last saved state

// invoking errors
#define rl_user_err(p, f, t, ...) rl_err((p), E_USER, (f), (t) __VA_OPT__(,) __VA_ARGS__)
#define rl_read_err(p, f, t, ...) rl_err((p), E_READ, (f), (t) __VA_OPT__(,) __VA_ARGS__)
#define rl_comp_err(p, f, t, ...) rl_err((p), E_COMP, (f), (t) __VA_OPT__(,) __VA_ARGS__)
#define rl_eval_err(p, f, t, ...) rl_err((p), E_EVAL, (f), (t) __VA_OPT__(,) __VA_ARGS__)
#define rl_runt_err(p, f, t, ...) rl_err((p), E_RUNT, (f), (t) __VA_OPT__(,) __VA_ARGS__)
#define rl_sys_err(p, f, t, ...)  rl_err((p), E_SYS,  (f), (t) __VA_OPT__(,) __VA_ARGS__)

int rl_err(Proc* p, Error e, char* f, char* t, ...) noreturn;

// error helpers
int    rl_require(Proc* p, bool c, Error e, char* f, char* t, ...);
size32 rl_argco(Proc* p, char* f, size32 g, bool v, size32 e);
size32 rl_argcos(Proc* p, char* f, size32 g, bool v, size32 n, ...);
size64 rl_bounds(Proc* p, char* f, size64 o, size64 l, size64 u);
Type   rl_argtype(Proc* p, char* f, Type g, Type e);
Type   rl_argtypes(Proc* p, char* f, Type g, size64 n, ...);

// safecast helpers
#define as_fn_s(p, n, x) generic2(as_fn_s, x, p, n, x)

Fn* val_as_fn_s(Proc* p, char* n, Val x);
Fn* obj_as_fn_s(Proc* p, char* n, void* x);

#endif
