#ifndef rl_vm_vm_h
#define rl_vm_vm_h

#include "common.h"

// Magic numbers
#define BUFFER_SIZE 2048
#define EXPR_STACK_SIZE 65536
#define CALL_STACK_SIZE 16384
#define ERROR_STACK_SIZE 512
#define BUFFER_MAX  2046
#define INIT_HEAP   (65536 * sizeof(uintptr_t))

// Internal types -------------------------------------------------------------
typedef struct CallState {
  int frame_size; /* stack usage */
  int cntl_off; /* offset to nearest enclosing control frame (0 if this is a control frame, -1 if no control frames exist) */
  int cntl_size; /* total size of stack enclosed by a control frame (-1 if no control frame exists) */
  int flags; /* idk but struct padding activates my autism */
  instr_t* savepc; /* caller's program counter */
  Method* savefn; /* caller's method */
} CallState;

// everything necessary to restore execution at a particular point
typedef struct ErrorState {
  Method* fn;
  instr_t* pc;
  int sp;
  int fp;
  int bp;
  jmp_buf Cstate;
} ErrorState;

/* global state object */
typedef struct RlVm {
  Env* globals; /* toplevel environment */
  Strings* strings; /* interned strings */
  size_t heap_cap, heap_used, gc_count;
  bool initialized, gc;
  Obj* managed_objects;
  Obj* permanent_objects;
  Objs* grays;
} RlVm;

/* thread state object */
struct RlState {
  RlVm* vm;
  UpVal* upvs;
  Method* fn;
  instr_t* pc;
  int sp, fp, bp, ep;
  CallState frames[CALL_STACK_SIZE];
  Expr stack[EXPR_STACK_SIZE];
  ErrorState err_states[ERROR_STACK_SIZE];
  char token[BUFFER_SIZE];
  size_t toff;
};

// forward declarations for global variables
extern char* ErrorNames[];
extern RlVm Vm;
extern RlState Main;
extern Port Ins, Outs, Errs;
extern char* CharNames[128];

// miscellaneous state helpers
char* current_fn_name(RlState* rls);

// token helpers
void reset_token(RlState* rls);
size_t add_to_token(RlState* rls, char c);

// stack helpers
void reset_stack(RlState* rls);
void reset_exec_state(RlState* rls);
void check_stack_limit(RlState* rls, int n);
int check_stack_bounds(RlState* rls, int n);
Expr* stack_ref(RlState* rls, int i);
int save_sp(RlState* rls);
void restore_sp(RlState* rls, int sp);
Expr* preserve(RlState* rls, int n, ...);
Expr*  push(RlState* rls, Expr x);
Expr*  dup(RlState* rls);
Expr*  pushn(RlState* rls, int n);
Expr   pop(RlState* rls);
Expr   rpop(RlState* rls);
Expr   popn(RlState* rls, int n);

// upval helpers
UpVal* get_upv(RlState* rls, Expr* loc);
void close_upvs(RlState* rls, Expr* base);

// frame helpers
void install_method(RlState* rls, Method* method, int argc);
bool save_call_frame(RlState* rls, int offset);
bool restore_call_frame(RlState* rls);
void reset_vm(RlState* rls);

// report helpers
void stack_report_slice(RlState* rls, int n, char* fmt, ...);
void stack_report(RlState* rls);
void env_report(RlState* rls);

// convenience macros
#define tos(rls) (stack_ref(rls, -1)[0])
#define next_op(rls) *((rls)->pc++)

#endif
