#ifndef rl_runtime_h
#define rl_runtime_h

#include "common.h"
#include "data.h"

// Internal types
typedef enum {
  OKAY,
  USER_ERROR,
  RUNTIME_ERROR,
  SYSTEM_ERROR
} Status;

typedef struct GcFrame {
  struct GcFrame* next;
  int count;
  Expr* exprs;
} GcFrame;

// Magic numbers
#define BUFFER_SIZE 2048
#define STACK_SIZE  65536
#define BUFFER_MAX  2046
#define INIT_HEAP   (4096 * sizeof(uintptr_t))

// forward declarations for global variables
extern char Token[BUFFER_SIZE];
extern size_t TOff;
extern Status VmStatus;
extern char* ErrorNames[];
extern jmp_buf Toplevel;
extern Obj* Heap;
extern size_t HeapUsed, HeapCap;
extern GcFrame* GcFrames;
extern Expr Stack[STACK_SIZE];
extern int Sp;

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
#define system_error(args...)  rascal_error(SYSTEM_ERROR, args)

#define tos()  stack_ref(-1)

#define preserve(n, vals...)                                            \
  Expr __gc_frame_vals__[(n)] = { vals };                               \
  GcFrame __gc_frame__ __attribute__((__cleanup__(next_gc_frame))) = {  \
    .next  = GcFrames,                                                  \
    .count =  (n),                                                      \
    .exprs = __gc_frame_vals__                                          \
  };                                                                    \
    GcFrames = &__gc_frame__

#endif
