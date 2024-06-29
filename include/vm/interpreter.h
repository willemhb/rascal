#ifndef rl_vm_interpreter_h
#define rl_vm_interpreter_h

#include "val/array.h"

/* Runtime interpreter state types and APIs */
/* C types */
struct IState {
  MVec stk; // ephemeral values
  MVec frm; // saved execution state

  // error flag
  rl_err_t err;

  // linked list of open upvalues
  UpVal* upv;

  /* Saved registers */
  Closure*  ex; // executing code object
  short*    ip; // instruction pointer
  int       fs; // size of the current frame
  int       co; // offset to frame saved by `catch` form
  int       ho; // offset to frame saved by `hndl` form
  int       sp; // value of stack pointer when last `catch` or `hndl` frame was installed (should be -1 for all other frames)

  /* volatile registers */
  Val*      bp; // stack allocated locals
};

/* External APIs */
/* IState APIs */
// initialization, &c
void reset_is(IState* s);
void init_is(IState* s, Closure* c);

// error handling
void is_panic(IState* s, rl_err_t e, const char* fmt, ...);
void is_vpanic(IState* s, rl_err_t e, const char* fmt, va_list va);
void is_chkargc(IState* s, rl_err_t e, char* fn, size_t x, size_t g, bool v);
void is_chktype(IState* s, rl_err_t e, char* fn, Type* x, Type* g);
void is_chkbnd(IState* s, rl_err_t e, char* fn, void* a, void* l, void* u);
void is_chklbnd(IState* s, rl_err_t e, char* fn, void* a, void* l);
void is_chkubnd(IState* s, rl_err_t e, char* fn, void* a, void* u);

// upvalues
UpVal* is_capture(IState* is, Val* l);
void   is_close(IState* is, Val* l);

// miscellaneous stack manipulation helpers
Val  is_tos(IState* s, bool c);
Val  is_peeps(IState* s, int i, bool c);
Val  is_peepf(IState* s, int i, bool c);
Val* is_peeks(IState* s, int i, bool c);
Val* is_peekf(IState* s, int i, bool c);
int  is_push(IState* s, Val x, bool c);
int  is_dup(IState* s, bool c);
int  is_pushn(IState* s, int n, Val x, bool c);
Val  is_pop(IState* s, bool c);
Val  is_popn(IState* s, int n, bool c);

// frame manipulation helpers
void is_pushf(IState* s, Closure* f, bool c);
void is_tpushf(IState* s, Closure* f, bool c);
void is_pushc(IState* s, Closure* f, Closure* h, bool c);
void is_pushh(IState* s, Closure* f, Closure* h, bool c);

void is_popf(IState* s, bool c);
void is_popc(IState* s, int n, bool c);
void is_poph(IState* s, int n, bool c);

#endif
