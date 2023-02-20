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
typedef struct {
  // heap state ---------------------------------------------------------------
  struct {
    usize allocated;
    usize next_gc;
    Obj*  live;
    Objs  grays;
  } heap;

  // error state --------------------------------------------------------------
  struct {
    Error error;
    Bin   buffer;
  } errors;

  // reader state -------------------------------------------------------------
  struct {
    Token  token;
    Vals   saved;
    Table  table;
    Bin    buffer;
  } reader;

  // interpreter & environment ------------------------------------------------
  struct {
    Table* globals;
    Val*   sp;
    Frame* fp;
  } interpreter;
} Vm;

// API ------------------------------------------------------------------------
// heap -----------------------------------------------------------------------
void* allocate(usize n, usize obsize, void* ini);
void* reallocate(void* ptr, usize n, usize obsize, void* ini);
void  deallocate(void* ptr, usize n, usize obsize);
void  manage(void);

// error ----------------------------------------------------------------------
bool panicking(void);
void panic(Error error, char* fmt, ...);
bool recover(void);

#define GUARD(test, sentinel, error, fmt, ...)              \
  do {                                                      \
    if(!(test)) {                                           \
      panic(error, fmt __VA_OPT__(,) __VA_ARGS__);          \
      return sentinel;                                      \
    }                                                       \
  } while (false)

// interpreter ----------------------------------------------------------------
Val*   push(Val x);
Val    pop(void);
Frame* pushf(Func* func, uint n);
Val    popf(void);

// initialization -------------------------------------------------------------
void runtime_init(void);

#endif
