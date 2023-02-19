#ifndef runtime_h
#define runtime_h

#include "value.h"

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

// virtual machine ------------------------------------------------------------
typedef struct {
  // heap state ---------------------------------------------------------------
  struct {
    usize allocated;
    usize next_gc;
    Obj*  live;
  } heap;

  // error state --------------------------------------------------------------
  struct {
    Error error;
    bool  panicking;
    Val   agitant;
    Bin*  buffer;
  } errors;

  // reader state -------------------------------------------------------------
  struct {
    Token  token;
    Val    value;
    Table* readtable;
    Bin*   buffer;
  } reader;

  
} Vm;



#endif
