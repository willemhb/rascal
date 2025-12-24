#ifndef rl_runtime_h
#define rl_runtime_h

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
typedef struct RlState {
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
} RlState;

// forward declarations for global variables
extern char* ErrorNames[];
extern RlVm Vm;
extern RlState Main;
extern Port Ins, Outs, Errs;
extern char* CharNames[128];

// function prototypes
// miscellaneous state helpers
char* current_fn_name(RlState* rls);

// handling error state
ErrorState* error_state(RlState* rls);
void save_error_state(RlState* rls);
void restore_error_state(RlState* rls);
void discard_error_state(RlState* rls);

#define set_safe_point(rls) (setjmp((rls)->err_states[(rls)->ep-1].Cstate))
#define rl_longjmp(rls, s) longjmp(error_state(rls)->Cstate, s)

// error function
void rascal_error(RlState* rls, Status etype, char* fmt, ...);

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

UpVal* get_upv(RlState* rls, Expr* loc);
void close_upvs(RlState* rls, Expr* base);

void install_method(RlState* rls, Method* method, int argc);
bool save_call_frame(RlState* rls, int offset);
bool restore_call_frame(RlState* rls);
void reset_vm(RlState* rls);

void add_to_managed(RlState* rls, void* ptr);
void add_to_permanent(RlState* rls, void* ptr);
void gc_save(RlState* rls, void* ob);
void run_gc(RlState* rls);
void heap_report(RlState* rls);
void stack_report_slice(RlState* rls, int n, char* fmt, ...);
void stack_report(RlState* rls);
void env_report(RlState* rls);

void* allocate(RlState* rls, size_t n);
char* duplicates(RlState* rls, char* cs);
void* duplicate(RlState* rls, size_t n, void* ptr);
void* reallocate(RlState* rls, size_t n, size_t o, void* spc);
void  release(RlState* rls, void* d, size_t n);

// convenience macros
#define user_error(rls, args...) rascal_error(rls, USER_ERROR, args)
#define runtime_error(rls, args...) rascal_error(rls, RUNTIME_ERROR, args)
#define eval_error(rls, args...) rascal_error(rls, EVAL_ERROR, args)
#define system_error(rls, args...) rascal_error(rls, SYSTEM_ERROR, args)

#define tos(rls) (stack_ref(rls, -1)[0])

#define next_op(rls) *((rls)->pc++)

#define require(rls, test, args...)             \
  do {                                          \
    if ( !(test) )                              \
      eval_error((rls), args);                  \
  } while ( false )

#define require_argco(rls, f, e, g)                                     \
  do {                                                                  \
    if ( (e) != (g) )                                                   \
      eval_error((rls), "%s wants %d inputs, got %d", (f), (e), (g));   \
  } while ( false )

#define require_argco2(rls, f, e1, e2, g)                               \
  do {                                                                  \
    if ( (e1) != (g) && (e2) != (g) )                                   \
      eval_error( (rls),                                                \
                  "%s wants %d or %d inputs, got %d",                   \
                  (f),                                                  \
                  (e1),                                                 \
                  (e2),                                                 \
                  (g));                                                 \
  } while ( false )

#define require_vargco(rls, f, e, g)                                    \
  do {                                                                  \
    if ( (g) < (e) )                                                    \
      eval_error( (rls), "%s wants at least %d inputs, got %d",         \
                  (f), (e), (g) );                                      \
  } while ( false )

#define require_argtype(rls, f, e, x)                                   \
  do {                                                                  \
    Type* _t = type_of(x);                                              \
    if ( (e)->tag != _t->tag )                                          \
      eval_error( (rls), "%s wanted a %s, got a %s",                    \
                  (f), type_name(e), type_name(_t));                    \
  } while ( false )

#define syntax_require(rls, sf, fn, t, fmt, ...)            \
  do {                                                      \
    if ( !(t) )                                             \
      eval_error( (rls),                                    \
                  "bad syntax for %s in %s: " fmt,          \
                  (sf), (fn) __VA_OPT__(,) __VA_ARGS__);    \
  } while ( false )

#define syntax_require_vargco(rls, sf, fn, e, f)                        \
  do {                                                                  \
    int __c = ((f) && (f)->tail) ? (int)((f)->tail->count) : -1;        \
    if ( __c < (e) )                                                    \
      eval_error( (rls),                                                \
                  "bad syntax for %s in %s: "                           \
                  "wanted at least %d expressions, got %d",             \
                  (sf), (fn), (e), __c );                               \
  } while ( false )

#define syntax_require_argco(rls, sf, fn, e, f)                         \
  do {                                                                  \
    int __c = ((f) && (f)->tail) ? (int)((f)->tail->count) : -1;        \
    if ( __c != (e) )                                                   \
      eval_error( (rls),                                                \
                  "bad syntax for %s in %s: "                           \
                  "wanted %d expressions, got %d",                      \
                  (sf), (fn), (e), __c );                               \
  } while ( false )

#define syntax_require_argco2(rls, sf, fn, e1, e2, f)                   \
  do {                                                                  \
    int __c = ((f) && (f)->tail) ? (int)((f)->tail->count) : -1;        \
    if ( __c != (e1) && __c != (e2) )                                   \
      eval_error( (rls),                                                \
                  "bad syntax for %s in %s: "                           \
                  "wanted %d or %d expressions, got %d",                \
                  (sf), (fn), (e1), (e2), __c );                        \
  } while ( false )

#endif
