#ifndef rl_runtime_h
#define rl_runtime_h

#include "common.h"

// Magic numbers
#define BUFFER_SIZE 2048
#define EXPR_STACK_SIZE 65536
#define CALL_STACK_SIZE 16384
#define BUFFER_MAX  2046
#define INIT_HEAP   (2048 * sizeof(uintptr_t))

// Internal types -------------------------------------------------------------
typedef enum {
  OKAY,
  USER_ERROR,
  EVAL_ERROR,
  RUNTIME_ERROR,
  SYSTEM_ERROR
} Status;

typedef struct GcFrame {
  struct GcFrame* next;
  int count;
  Expr* exprs;
} GcFrame;

typedef struct CallState {
  int frame_size; /* stack usage */
  int cntl_off; /* offset to nearest enclosing control frame (0 if this is a control frame, -1 if no control frames exist) */
  int cntl_size; /* total size of stack enclosed by a control frame (-1 if no control frame exists) */
  int flags; /* idk but struct padding activates my autism */
  instr_t* savepc; /* caller's program counter */
} CallState;

/* global state object */
typedef struct RlVm {
  Env* globals; /* toplevel environment */
  Strings* strings; /* interned strings */
  size_t heap_cap, heap_used;
  Obj* heap_live;
  Objs* grays;
} RlVm;

/* thread state object */
typedef struct RlState {
  RlVm* vm;
  UpVal* upvs;
  Fun* fn;
  instr_t* pc;
  int sp, fp, bp, ep;
  CallState frames[CALL_STACK_SIZE];
  Expr stack[EXPR_STACK_SIZE];
} RlState;

// everything necessary to restore execution at a particular point
typedef struct VmCtx {
  GcFrame* gcf;
  Fun* fn;
  instr_t* pc;
  int sp;
  int fp;
  int bp;
  jmp_buf Cstate;
} VmCtx;

extern VmCtx SaveStates[MAX_SAVESTATES];
extern int ep;

#define SaveState (SaveStates[ep-1])

// forward declarations for global variables
extern char Token[BUFFER_SIZE];
extern size_t TOff;
extern char* ErrorNames[];
extern GcFrame* GcFrames;
extern Env Globals;
extern RlVm Vm;
extern RlState Main;
extern Port Ins, Outs, Errs;
extern char* CharNames[128];

// function prototypes
void   panic(Status etype);
void   recover(void);
void   rascal_error(Status etype, char* fmt, ...);

void   reset_token(void);
size_t add_to_token(char c);

void   reset_stack(RlState* rls);
Expr*  stack_ref(RlState* rls, int i);
Expr*  push(RlState* rls, Expr x);
Expr*  pushn(RlState* rls, int n);
Expr   pop(RlState* rls);
Expr   rpop(RlState* rls);
Expr   popn(RlState* rls, int n);

UpVal* get_upv(RlState* rls, Expr* loc);
void   close_upvs(RlState* rls, Expr* base);

void   install_fun(RlState* rls, Fun* fun, int argc);
void   save_frame(RlState* rls);
void   restore_frame(RlState* rls);
void   reset_vm(RlState* rls);

void   add_to_heap(RlState* rls, void* ptr);
void   gc_save(RlState* rls, void* ob);
void   run_gc(RlState* rls);
void   heap_report(RlState* rls);
void   stack_report(RlState* rls);
void   env_report(RlState* rls);

void   save_ctx(RlState* rls);    // save execution state
void   restore_ctx(RlState* rls); // restore saved execution state
void   discard_ctx(void); // discard saved execution state (presumably because we're exiting normally)

void*  allocate(RlState* rls, size_t n);
char*  duplicates(RlState* rls, char* cs);
void*  duplicate(RlState* rls, size_t n, void* ptr);
void*  reallocate(RlState* rls, size_t n, size_t o, void* spc);
void   release(RlState* rls, void* d, size_t n);
void   next_gc_frame(GcFrame* gcf);

// convenience macros
#define safepoint() setjmp(SaveState.Cstate)

#define user_error(args...)    rascal_error(USER_ERROR, args)
#define runtime_error(args...) rascal_error(RUNTIME_ERROR, args)
#define eval_error(args...)    rascal_error(EVAL_ERROR, args)
#define system_error(args...)  rascal_error(SYSTEM_ERROR, args)

#define tos(rls)  (stack_ref(rls, -1)[0])

#define next_op(rls) *((rls)->pc++)

#define preserve(n, vals...)                                            \
  Expr __gc_frame_vals__[(n)] = { vals };                               \
  GcFrame __gc_frame__ __attribute__((__cleanup__(next_gc_frame))) = {  \
    .next  = GcFrames,                                                  \
    .count =  (n),                                                      \
    .exprs = __gc_frame_vals__                                          \
  };                                                                    \
    GcFrames = &__gc_frame__

#define add_to_preserved(n, x)                  \
  __gc_frame_vals__[(n)] = (x)

#define preserved() __gc_frame_vals__

#define require(test, args...)                  \
  do {                                          \
    if ( !(test) )                              \
      eval_error(args);                         \
  } while ( false )

#define require_argco(f, e, g)                                          \
  do {                                                                  \
    if ( (e) != (g) )                                                   \
      eval_error( "%s wants %d inputs, got %d", (f), (e), (g) );        \
  } while ( false )

#define require_argco2(f, e1, e2, g)                                    \
  do {                                                                  \
    if ( (e1) != (g) && (e2) != (g) )                                   \
      eval_error( "%s wants %d or %d inputs, got %d",                   \
                  (f),                                                  \
                  (e1),                                                 \
                  (e2),                                                 \
                  (g));                                                 \
  } while ( false )

#define require_vargco(f, e, g)                                         \
  do {                                                                  \
    if ( (g) < (e) )                                                    \
      eval_error( "%s wants at least %d inputs, got %d",                \
                  (f), (e), (g) );                                      \
  } while ( false )

#define require_argtype(f, e, x)                                        \
  do {                                                                  \
    ExpType _t = exp_type(x);                                           \
    if ( (e) != _t )                                                    \
      eval_error("%s wanted a %s, got a %s",                            \
                 (f), Types[e].name, Types[_t].name);                   \
  } while ( false )

#endif
