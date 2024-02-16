#ifndef rl_vm_interpreter_h
#define rl_vm_interpreter_h

#include "val/value.h"

/* Call stack and Value stack API. */

/* C types */
struct CallFrame {
  /* Executing code. */
  Closure* code;

  /* basic pointers */
  int ip, bp, sp;

  /* saved handler state

     app - active prompt pointer.
     aap - active abort pointer.
     ppp - parent prompt pointer.
     pap - parent abort pointer. */
  int app, aap, ppp, pap;
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
  CallFrame* fp, * app, * aap, * ppp, * pap;
};

/* Globals */
extern RlInterpreter Interpreter;

/* External API */

/* Initialization. */
void vm_init_interpreter(void);

#endif
