#ifndef rl_runtime_h
#define rl_runtime_h

#include "common.h"
#include "data.h"

// Magic numbers
#define BUFFER_SIZE 2048
#define STACK_SIZE  65536
#define BUFFER_MAX  2046
#define INIT_HEAP   (2048 * sizeof(uintptr_t))

// Internal types
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

typedef struct {
  Fun*     fn;
  instr_t* pc;
  int      sp;
  int      fp;
  int      bp;
  Expr     stack[STACK_SIZE];
} VM;

// forward declarations for global variables
extern char Token[BUFFER_SIZE];
extern size_t TOff;
extern Status VmStatus;
extern char* ErrorNames[];
extern jmp_buf Toplevel;
extern Obj* Heap;
extern size_t HeapUsed, HeapCap;
extern GcFrame* GcFrames;
extern Env Globals;
extern VM Vm;

// function prototypes
void   panic(Status etype);
void   recover(void);
void   rascal_error(Status etype, char* fmt, ...);
void   reset_token(void);
size_t add_to_token(char c);
void   reset_stack(void);
Expr*  stack_ref(int i);
Expr*  push(Expr x);
Expr*  pushn(int n);
Expr   pop(void);
Expr   popn(int n);
void   install_fun(Fun* fun, int bp, int fp);
void   save_frame(void);
void   restore_frame(void);
void   reset_vm(void);
void   gc_save(void* ob);
void   run_gc(void);
void*  allocate(bool h, size_t n);
char*  duplicates(char* cs);
void*  reallocate(bool h, size_t n, size_t o, void* spc);
void   release(void* d, size_t n);
void   next_gc_frame(GcFrame* gcf);

// convenience macros
#define safepoint() setjmp(Toplevel)

#define user_error(args...)    rascal_error(USER_ERROR, args)
#define runtime_error(args...) rascal_error(RUNTIME_ERROR, args)
#define eval_error(args...)    rascal_error(EVAL_ERROR, args)
#define system_error(args...)  rascal_error(SYSTEM_ERROR, args)

#define tos()  (stack_ref(-1)[0])

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
