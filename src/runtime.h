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

typedef struct {
  Func*   func;
  uint16* ip;
  Val*    slots;
  bool    captured;
} Frame;

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
    Table  globals;
    Val*   sp;
    Frame* fp;
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
  do {						\
    if (panicking())				\
      return sentinel;				\
  } while (false)

// interpreter ----------------------------------------------------------------
Val*   push(Val x);
Val    pop(void);
Val*   peep(int i);
Val    peek(int i);

Frame* pushf(Func* func, uint n);
Val    popf(void);

// initialization -------------------------------------------------------------
void runtime_init(void);

#endif
