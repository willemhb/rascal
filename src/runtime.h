#ifndef runtime_h
#define runtime_h

#include "value.h"
#include "object.h"

/* global runtime state & API */

// C types --------------------------------------------------------------------
typedef enum {
  NO_ERROR,
  READ_ERROR,
  COMPILE_ERROR,
  EVAL_ERROR,
  APPLY_ERROR
} Error;

typedef enum {
  READY_TOKEN,
  EXPRESSION_TOKEN,
  EOS_TOKEN,
  ERROR_TOKEN
} Token;

typedef struct Frame Frame;

struct Frame {
  Frame*  prompt; // nearest enclosing frame added by a `with` form
  Func*   func;   // executing code object
  uint16* ip;     // instruction pointer
  Val*    slots;
};

// virtual machine ------------------------------------------------------------
struct Vm {
  // heap state ---------------------------------------------------------------
  struct HeapData {
    usize allocated;
    usize next_gc;
    Obj*  live;
    Objs  grays;
  } heap;

  // error state --------------------------------------------------------------
  struct ErrorData {
    Error error;
    Bin   buffer;
  } errors;

  // reader state -------------------------------------------------------------
  struct ReaderData {
    Token  token;
    Vals   saved;
    Table  table;
    Bin    buffer;
  } reader;

  // interpreter & environment ------------------------------------------------
  struct InterpreterData {
    Val*   sp,* sb,* se;
    Frame* fp,* fb,* fe;
  } interpreter;
};

// globals --------------------------------------------------------------------
extern struct Vm Vm;

#define Heap        (Vm.heap)
#define Reader      (Vm.reader)
#define Interpreter (Vm.interpreter)
#define Errors      (Vm.errors)

// API ------------------------------------------------------------------------
// heap -----------------------------------------------------------------------
void  register_obj(void* obj);
void* allocate(uint n, usize obsize, uint64 ini);
void* reallocate(void* ptr, uint new, uint old, usize obsize, uint64 ini);
void  deallocate(void* ptr, uint n, usize obsize);
void  manage(void);

// error ----------------------------------------------------------------------
bool  panicking(void);
void  panic(Error error, char* fmt, ...);
Error recover(void);

#define GUARD(test, sentinel, error, fmt, ...)              \
  do {                                                      \
    if(!(test)) {                                           \
      panic(error, fmt __VA_OPT__(,) __VA_ARGS__);          \
      return sentinel;                                      \
    }                                                       \
  } while (false)

#define REPANIC(sentinel)			\
  do {                              \
    if (panicking())				\
      return sentinel;				\
  } while (false)

// interpreter ----------------------------------------------------------------
#define FPRX     (Interpreter.fp)
#define SPRX     (Interpreter.sp)
#define TOSRX    (SPRX[-1])
#define FRAMERX  (FPRX[-1])
#define FUNCRX   (FRAMERX.func)
#define CHUNKRX  ((Chunk*)FUNCRX->func)
#define IPRX     (FRAMERX.ip)
#define SLOTSRX  (FRAMERX.slots)
#define PROMPTRX (FRAMERX.prompt)
#define VALSRX   (CHUNKRX->vals->array)
#define LENVRX   (CHUNKRX->lenv)
#define CENVRX   (CHUNKRX->cenv)
#define CODERX   (CHUNKRX->code)

Val*   push(Val x);
Val    pop(void);
Val*   peep(int i);
Val    peek(int i);

Frame* pushf(Func* func, uint n);
Val    popf(void);

void   reset_sp(void);
void   reset_fp(void);

// initialization -------------------------------------------------------------
void runtime_init(void);

#endif
