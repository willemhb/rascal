#ifndef rl_vm_vm_h
#define rl_vm_vm_h

#include "common.h"
#include "util/collection.h"

// Magic numbers
#define STACK_SIZE 65536
#define NUM_FRAMES 2048
#define BUFFER_SIZE 2048
#define INIT_HEAP  (65536 * sizeof(uintptr_t))

// Internal types -------------------------------------------------------------
/* global state object */
typedef struct RlVm {
  Env* globals; /* toplevel environment */
  Strings strings; /* interned strings */

  /* miscellaneous */
  bool debug;

  /* heap state */
  size_t heap_cap, heap_used, gc_count;
  bool initialized, gc;
  Obj* managed_objects;
  Obj* permanent_objects;
  Objs grays;
} RlVm;

/* represents a single call frame */
struct Frame {
  Method* exec;
  instr_t* pc;
  StackRef base;
  StackRef esc;
};

/* thread state object */
struct RlState {
  RlVm* vm;
  UpVal* upvs;

  /* exeuction state */
  Method* exec;
  instr_t* pc;
  /* stack pointers */
  StackRef const stack, s_end; // these shouldn't change
  StackRef s_top, base, esc;
  /* frame pointers */
  FrameRef const frames, f_end;
  FrameRef f_top;
  /* reader state */
  TextBuf token;
};

// forward declarations for global variables
extern RlVm Vm;
extern RlState Main;
extern Port Ins, Outs, Errs;

// token helpers
void reset_token(RlState* rls);
int add_to_token(RlState* rls, char c);
char* token_val(RlState* rls);
int token_size(RlState* rls);

// stack helpers
int stack_size(RlState* rls);
int call_stack_size(RlState* rls);
void stack_check_limit(RlState* rls, int n);
int stack_check_bounds(RlState* rls, int n);
void frames_check_limit(RlState* rls, int n);
void stack_swap(RlState* rls, int ox, int oy);
Expr* stack_preserve(RlState* rls, int n, ...);
Expr* stack_push(RlState* rls, Expr x);
Expr* stack_dup(RlState* rls);
Expr* stack_pushn(RlState* rls, int n);
Expr stack_pop(RlState* rls);
Expr stack_popn(RlState* rls, int n);
Expr stack_rpop(RlState* rls);
Expr stack_rpopn(RlState* rls, int n);
void stack_swap_s(RlState* rls, int ox, int oy);
Expr* stack_preserve_s(RlState* rls, int n, ...);
Expr* stack_push_s(RlState* rls, Expr x);
Expr* stack_dup_s(RlState* rls);
Expr* stack_pushn_s(RlState* rls, int n);
Expr stack_pop_s(RlState* rls);
Expr stack_popn_s(RlState* rls, int n);
Expr stack_rpop_s(RlState* rls);
Expr stack_rpopn_s(RlState* rls, int n);

// upval helpers
UpVal* get_upv(RlState* rls, Expr* loc);
void close_upvs(RlState* rls, Expr* base);

// frame helpers
void install_method(RlState* rls, Method* method, int argc);
void save_call_frame(RlState* rls);
void save_call_frame_s(RlState* rls);
void restore_call_frame(RlState* rls);
void restore_call_frame_s(RlState* rls);

// report helpers
void stack_report(RlState* rls, int n, char* fmt, ...);
void print_call_stack(RlState* rls, int n, char* fmt, ...);
void env_report(RlState* rls, Env* vars);

// convenience macros
#define tos(rls)  ((rls)->s_top[-1])
#define next_op(rls) *((rls)->pc++)

#endif
