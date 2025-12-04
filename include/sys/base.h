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
// internal collection types (APIs are in internals.h)
struct IStack {
  bool tagged;
  lcount_t count, max_count;
  void* vals;
};

typedef struct {
  Expr key;
  Expr val;
} TEntry;

struct ITable {
  bool environ; /* Special constraints on keys and values if this is true */
  lcount_t count, max_count;
  TEntry* kvs;
};

struct IBuffer {
  int elsize;
  bool encoded;
  lcount_t count, max_count;
  void* data;
};

typedef struct {
  char* key;
  Sym* val;
} STEntry;

struct SymbolTable {
  lcount_t count, max_count;

  STEntry* symbols;
};

// saved call state (stack frame)
struct CallState {
  int frame_size; /* stack usage */
  int cntl_off; /* offset to nearest enclosing control frame (0 if this is a control frame, -1 if no control frames exist) */
  int cntl_size; /* total size of stack enclosed by a control frame (-1 if no control frame exists) */
  int flags; /* idk but struct padding activates my autism */
  instr_t* savepc; /* caller's program counter */
};

struct GlobalState {
  ITable globals; /* global namespace */

  /* heap state */
  size_t heap_used; /* amount of heap space actually used */
  size_t heap_total; /* amount of heap space available */
  Obj* gcroot; /* list of all collectable objects */
  IStack grays; /* stack of objects that are live but untraced */
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
// extern char* ErrorNames[NUM_ERRORS];
// extern Sym* ErrorTypes[NUM_ERRORS];

extern GlobalState Vm;
extern RascalState Main;

// function prototypes --------------------------------------------------------
// VM helpers -----------------------------------------------------------------
void reset_stack(void);
void reset_vm(void);

// initialization -------------------------------------------------------------

#endif
