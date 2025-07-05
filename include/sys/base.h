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
#define N_VALS      65536
#define BUFFER_MAX  2046
#define INIT_HEAP   (2048 * sizeof(uptr_t))

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
  int sp, fp, bp, cp; // stack pointer, frame pointer, base pointer, catch pointer
  Expr* stack;
} VM;

// globals --------------------------------------------------------------------
// Error state
extern jmp_buf SaveState;
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
// VM helpers -----------------------------------------------------------------
void reset_stack(void);
void reset_vm(void);

// reader helpers -------------------------------------------------------------
void   reset_token(void);
size_t add_to_token(char c);

// initialization -------------------------------------------------------------

#endif
