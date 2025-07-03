#ifndef rl_sys_base_h
#define rl_sys_base_h

/**
 *
 * Common declarations, APIs, and helpers for Rascal runtime system.
 *
 **/

// headers --------------------------------------------------------------------
#include "common.h"

// macros ---------------------------------------------------------------------
// Magic numbers
#define BUFFER_SIZE 2048
#define N_FRAMES    32768
#define N_VALS      65536
#define BUFFER_MAX  2046
#define EFRAME_SIZE 7
#define INIT_HEAP   (2048 * sizeof(uptr_t))

// Miscellaneous aliases
#define SaveState (SaveStates[ep-1])

// C types --------------------------------------------------------------------
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

// globals --------------------------------------------------------------------
// Error state
extern VmCtx SaveStates[MAX_SAVESTATES];
extern int ep;
extern char* ErrorNames[NUM_ERRORS];
extern Sym* ErrorTypes[NUM_ERRORS];

// Heap state
extern Obj* Heap;
extern size_t HeapUsed, HeapCap;
extern GcFrame* GcFrames;

// Interpreter state
extern VM Vm;

// Reader state
extern char Token[BUFFER_SIZE];
extern size_t TOff;

// function prototypes --------------------------------------------------------
// managing C execution context -----------------------------------------------
void save_ctx(void);    // save C execution state
void restore_ctx(void); // restore saved execution state
void discard_ctx(void); // discard save point

// memory helpers -------------------------------------------------------------
void  add_to_heap(void* ptr);
void  gc_save(void* ob);
void  run_gc(void);
void  heap_report(void);

// Rascal reader internals ----------------------------------------------------
void   reset_token(void);
size_t add_to_token(char c);

// initialization -------------------------------------------------------------

#endif
