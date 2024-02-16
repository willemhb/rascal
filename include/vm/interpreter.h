#ifndef rl_vm_interpreter_h
#define rl_vm_interpreter_h

#include "val/value.h"

/* Call stack and Value stack API. */

/* C types */
struct CallFrame {
  /* Executing code. */
  Closure* code;

  /* instruction pointer, base pointer, prompt pointer, abort pointer */
  int ip, bp, pp, ap;
};

struct RlInterpreter {
  /* open upvalues. */
  UpValue* upvals;

  /* Executing code. */
  Closure* code;

  /* instruction pointer. */
  uint16_t* ip;

  /* base and stack pointers. */
  Value* bp, * sp;

  /* call stack and handle pointers. */
  CallFrame* fp, * pp, * ap;
};

/* Globals */
extern RlInterpreter Interpreter;

/* External API */


#endif
