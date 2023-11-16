#ifndef rascal_runtime_h
#define rascal_runtime_h

#include "array.h"

/* C types and APIs for the runtime system, including global state, memory management, and internal error handling. */

// C types
/* stores values on the C stack that need to be preserved during garbage collection. */
struct GcFrame {
  GcFrame* next;
  size_t   cnt;
  Value*   vals;
};

/* compiler frame */
struct CompFrame {
  CompState    state; // current compiler state (distinguishes what type of chunk is being compiled)
  flags_t      flags; // miscellaneous flags
  char*        name;
  Chunk*       code;  // compiling code object
  Environment* envt;  // naming context
  Handlers*    hndl;  // effect handlers
};

/* reader frame */
struct ReadFrame {
  ReadState state;      // current reader state
  flags_t   flags;      // miscellaneous flags
  Table*    table;      // current readtable
  Stream*   source;     // input source
  Buffer8   buffer;
};

/* execution frame */
struct ExecFrame {
  Closure*  code;       // executing code object
  uint16_t* ip;         // current instruction pointer
  size_t    bp;         // stack address of current closure arguments
  size_t    fp;         // address of caller's frame
  size_t    pp;         // address of nearest enclosing prompt
};

/* exception context */
struct Context {
  Context* next;

  // gc, reader, compiler, and execution context
  GcFrame*   g_frame;
  ReadFrame* r_frame;
  CompFrame* c_frame;
  ExecFrame* e_frame;

  // C execution state
  jmp_buf buf;
};

/* Composite global state object. */
struct Vm {
  // heap
  struct {
    Obj*     objs;
    size_t   used;
    size_t   cap;
    GcFrame* frame;   // chain of values on the C stack that need to be preserved during collection
    Objects  stack;   // stack of marked values waiting to be traced
  } h;

  // environment
  struct {
    SymbolTable* symbols;
    NameSpace*   globals;
    Table*       annot;
    Table*       used;
  } n;

  // reader state
  struct {
    Table*     table;   // toplevel readtable
    ReadFrame* frame;   // pointer to current reader frame
    Alist      stack;   // private stack for storing subexpressions during `read`
  } r;

  // compiler state
  struct {
    CompFrame* frame;  // pointer to current compiler frame
    Alist      stack;  // private stack for storing subexpressions during `compile`
  } c;

  // execution state
  struct {
    UpValue*   upvals;  // toplevel list of open upvalues
    ExecFrame* frame;   // current exec frame
    size_t     sp;      // stack pointer
  } e;

  // miscellaneous state
  struct {
    Context* ctx;  // error recovery context
    bool     init; // indicates whether the VM has finished initializing
  } m;
};

/* miscellaneous object types vital in the runtime system */
/* represents a reified continuation. */
struct Control {
  Obj        obj;
  Value*     stack_copy;
  ExecFrame* frames_copy;
  size_t     n_stack, n_frames;
};

/* holds an upvalue (or its address). */
struct UpValue {
  Obj      obj;
  UpValue* next;
  size_t   offset;
  Value    value;
};

// globals
#define N_VALUES 32768
#define N_EXEC   8192
#define N_READ   2048
#define N_COMP   2048

extern Vm RlVm;

extern ReadFrame ReadFrames[N_READ];
extern CompFrame CompFrames[N_COMP];
extern ExecFrame ExecFrames[N_EXEC];
extern Value     StackSpace[N_VALUES];

#define STACK_BASE (&StackSpace[0])
#define STACK_END  (&StackSpace[N_VALUES])

// external API
void init_vm(Vm* vm);
void free_vm(Vm* vm);
void sync_vm(Vm* vm);

size_t push_macro_args(List* form);
size_t push(Value x);
Value  pop(void);
size_t pushn(size_t n);
Value  popn(size_t n);
Value* peek(int i);

#endif
