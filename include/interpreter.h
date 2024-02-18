#ifndef rl_interpreter_h
#define rl_interpreter_h

#include "value.h"

/* Definitions and declarations for virtual machine internals (execution of rascal code). */

extern struct {
  
  Value* sp;
} Interpreter;

/* API */
void  push(Value x);
void  pushn(size_t n, ...);
Value pop(void);
void  popn(size_t n);

#endif
