#ifndef rl_runtime_h
#define rl_runtime_h

#include "common.h"

// Magic numbers
#define BUFFER_SIZE 2048
#define N_FRAMES    32768
#define N_VALS      65536
#define BUFFER_MAX  2046
#define EFRAME_SIZE 7
#define INIT_HEAP   (2048 * sizeof(uintptr_t))

// Internal types -------------------------------------------------------------
typedef enum {
  OKAY,
  USER_ERROR,
  EVAL_ERROR,
  RUNTIME_ERROR,
  SYSTEM_ERROR
} Status;

#define NUM_ERRORS (SYSTEM_ERROR+1)

typedef struct GcFrame {
  struct GcFrame* next;
  int count;
  Expr* exprs;
} GcFrame;

typedef struct {
  UpVal* upvs;
  Fun* fn;
  instr_t* pc;
  int sp, fp, bp;
  Expr* frames, * vals;
} VM;

// C state information necessary for error handling
typedef struct VmCtx {
  GcFrame* gcf;
  Fun* fn;
  instr_t* pc;
  int sp, fp, bp;
  jmp_buf Cstate;
} VmCtx;

extern VmCtx SaveStates[MAX_SAVESTATES];
extern int ep;

#define SaveState (SaveStates[ep-1])

// forward declarations for global variables
extern char Token[BUFFER_SIZE];
extern size_t TOff;
extern char* ErrorNames[NUM_ERRORS];
extern Sym* ErrorTypes[NUM_ERRORS];
extern Obj* Heap;
extern size_t HeapUsed, HeapCap;
extern GcFrame* GcFrames;
extern Env Globals;
extern VM Vm;
extern Port Ins, Outs, Errs;
extern char* CharNames[128];

// function prototypes
void   panic(Status etype);
void   recover(funcptr_t cleanup);
void   rascal_error(Status etype, char* fmt, ...);

void   reset_token(void);
size_t add_to_token(char c);

// stack & frame operations
void   reset_vals(void);
Expr*  vals_ref(int i);
void   setvp(int n);
Expr*  vpush(Expr x);
Expr*  vrpush(Expr x);
Expr*  vpushn(int n);
Expr   vpop(void);
Expr   vrpop(void);
Expr   vpopn(int n);

void   reset_frames(void);
Expr*  frames_ref(int i);
void   setfp(int n);
Expr*  fpush(Expr x);
Expr*  fpushn(int n);
Expr   fpop(void);
Expr   frpop(void);
Expr   fpopn(int n);

// frame manipulation
void   install_fun(Fun* fun, int bp);
void   save_frame(void);
void   restore_frame(void);

// other vm stuff
void   reset_vm(void);
UpVal* get_upv(Expr* loc);
void   close_upvs(Expr* base);

void  add_to_heap(void* ptr);
void  gc_save(void* ob);
void  run_gc(void);
void  heap_report(void);

void save_ctx(void);    // save C execution state
void restore_ctx(void); // restore saved execution state
void discard_ctx(void); // discard save point

void*  allocate(bool h, size_t n);
char*  duplicates(char* cs);
void*  duplicate(bool h, size_t n, void* ptr);
void*  reallocate(bool h, size_t n, size_t o, void* spc);
void   release(void* d, size_t n);
void   next_gc_frame(GcFrame* gcf);

// convenience macros
#define safepoint() setjmp(SaveState.Cstate)

#define user_error(args...)    rascal_error(USER_ERROR, args)
#define runtime_error(args...) rascal_error(RUNTIME_ERROR, args)
#define eval_error(args...)    rascal_error(EVAL_ERROR, args)
#define system_error(args...)  rascal_error(SYSTEM_ERROR, args)

#define tos()  (vals_ref(-1)[0])

#define next_op() *(Vm.pc++)

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
