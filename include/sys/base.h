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

struct CallState {
  StackRef base;
  StackRef func;
  StackRef top;
  CSRef    catch;
  instr_t* savepc;
};

struct GlobalState {
  StringsTable* strings; /* interned strings */
  Env* globals; /* global namespace */

  /* heap state */
  size_t heap_used; /* amount of heap space actually used */
  size_t heap_total; /* amount of heap space available */
  Obj* gcroot; /* list of all collectable objects */
  Objects* gray; /* stack of objects that are live but untraced */
};

struct RascalState {
  GlobalState* global_state; /* link back to owning global state */

  /* interpreter state */
  UpVal* upvalues; /* list of all open upvalues in the current thread */
  Fun* function; /* currently executing user function */
  instr_t* pc; /* program counter */
  StackRef top; /* stack pointer */
  StackRef base; /* base pointer (current function) */
  StackRef stack; /* expression stack */
  CSRef cs_top; /* top of call state stack */
  CSRef cs_stack; /* call state stack */
};


// globals --------------------------------------------------------------------
// Error state
extern jmp_buf SaveState;
extern char* ErrorNames[NUM_ERRORS];
extern Sym* ErrorTypes[NUM_ERRORS];

// function prototypes --------------------------------------------------------
// VM helpers -----------------------------------------------------------------
void reset_stack(void);
void reset_vm(void);

// reader helpers -------------------------------------------------------------
void   reset_token(void);
size_t add_to_token(char c);

// initialization -------------------------------------------------------------

#endif
